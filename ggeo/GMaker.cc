/*
 * Copyright (c) 2019 Opticks Team. All Rights Reserved.
 *
 * This file is part of Opticks
 * (see https://bitbucket.org/simoncblyth/opticks).
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */


// npy-
#include "NGLM.hpp"
#include "GLMFormat.hpp"
#include "NPY.hpp"
#include "NTrianglesNPY.hpp"
#include "NCSG.hpp"


#include "NSphere.hpp"
#include "NPlane.hpp"
#include "NPrism.hpp"
#include "NPart.hpp"

#include "OpticksCSG.h"

// ggeo-
#include "GBndLib.hh"

#include "GParts.hh"
#include "GPt.hh"

#include "GBBoxMesh.hh"
#include "GMesh.hh"
#include "GMeshLib.hh"
#include "GMeshMaker.hh"
#include "GVolume.hh"
#include "GVector.hh"
#include "GMatrix.hh"
#include "GMaker.hh"

#include "SLOG.hh"


const plog::Severity GMaker::LEVEL = debug ; 



GMaker::GMaker(Opticks* ok, GBndLib* blib, GMeshLib* meshlib)
    :
    m_ok(ok),
    m_bndlib(blib),   // required for adding boundaries and returning indices for them 
    m_meshlib(meshlib)
{
    init();
}


void GMaker::init()
{
    assert( m_bndlib );
}


GVolume* GMaker::make(unsigned int /*index*/, int type, glm::vec4& param, const char* spec )
{
    // invoked from eg GGeoTest::createBoxInBox while looping over configured shape/boundary/param entries
    // for CSG triangulation need to be given the tree.. 
    //
    // NB note the the move to SDF means that forming a polygonization
    // no longer requires this, instead must implement the SDF : signed distance function

     GVolume* volume = NULL ; 
     switch(type)
     {
         case CSG_BOX:          volume = makeBox(param); break;
         case CSG_PRISM:        volume = makePrism(param, spec); break;
         case CSG_SPHERE:       volume = makeSubdivSphere(param, 3, "I") ; break; // I:icosahedron O:octahedron HO:hemi-octahedron C:cube 
         case CSG_ZSPHERE:      volume = makeZSphere(param) ; break;
         case CSG_ZLENS:        volume = makeZSphereIntersect_DEAD(param, spec) ; break;   // composite handled by adding child node
         case CSG_INTERSECTION: volume = makeBox(param); break ;    // boolean intersect
         case CSG_UNION:        volume = makeBox(param); break ;    // boolean union
         case CSG_DIFFERENCE:   volume = makeBox(param); break ;    // boolean difference
         default:               volume = nullptr       ; break ;
     }
     assert(volume);
     volume->setCSGFlag( type );

     // TODO: most parts alread hooked up above, do this uniformly
     GParts* pts = volume->getParts();  
     if(pts == NULL)
     {
         pts = GParts::Make(type, param, spec);  // (1,4,4) with typecode and bbox set 
         volume->setParts(pts);
     }
     assert(pts);

     unsigned boundary = m_bndlib->addBoundary(spec);  // only adds if not existing
     volume->setBoundaryAll(boundary);   // All loops over immediate children, needed for composite
     pts->setBoundaryAll(boundary);
     pts->enlargeBBoxAll(0.01f );

     return volume ; 
}



std::string GMaker::LVName(const char* shapename, int idx)
{
    return NCSG::TestVolumeName(shapename, "lv", idx );
}

std::string GMaker::PVName(const char* shapename, int idx)
{
    return NCSG::TestVolumeName(shapename, "pv", idx );
}





/**
GMaker::makeMeshFromCSG
----------------------

Hmm : this is using my (very temperamental) polygonization,
but there is no need to do so in direct workflow as the Geant4 
polygonization GMesh is available. 

Still used from GGeoTest which gets its geometry from the 
analytic python description with no Geant4 in sight.

**/


GMesh* GMaker::makeMeshFromCSG( NCSG* csg ) // cannot be const NCSG due to lazy NCSG::polgonize 
{
    unsigned index = csg->getIndex();
    const char* spec = csg->getBoundary();  
    NTrianglesNPY* tris = csg->polygonize();

    LOG(LEVEL) 
        << " index " << index 
        << " spec " << spec 
        << " numTris " << ( tris ? tris->getNumTriangles() : 0 )
        << " trisMsg " << ( tris ? tris->getMessage() : "" )
        ; 

    GMesh* mesh = GMeshMaker::Make(tris->getTris(), index);
    mesh->setCSG(csg);
    return mesh ; 
}


GVolume* GMaker::makeVolumeFromMesh( unsigned ndIdx, const GMesh* mesh ) const 
{
    glm::mat4 txf(1.0f); 
    return makeVolumeFromMesh( ndIdx, mesh, txf ); 
}

GVolume* GMaker::makeVolumeFromMesh( unsigned ndIdx, const GMesh* mesh, const glm::mat4& txf ) const 
{
    const NCSG* csg = mesh->getCSG();   

    unsigned lvIdx = mesh->getIndex(); 

    const char* spec = csg->getBoundary();  

    GMatrixF* transform = new GMatrix<float>(glm::value_ptr(txf));

    GVolume* volume = new GVolume( ndIdx, transform, mesh, NULL, -1 );     
    // csg is mesh-qty not a node-qty, boundary spec is a node-qty : so this is just for testing

    int type = csg->getRootType() ;

    std::string lvn = csg->getTestLVName();
    std::string pvn = csg->getTestPVName();
    
    volume->setPVName( strdup(pvn.c_str()) );
    volume->setLVName( strdup(lvn.c_str()) );
    volume->setCSGFlag( type );


#ifdef GPARTS_OLD
    GParts* parts = GParts::Make( csg, spec, ndIdx );  
    volume->setParts( parts );
#else
    unsigned csgIdx = csg->getIndex(); 
    GPt* pt = new GPt( lvIdx, ndIdx, csgIdx, spec )  ;
    volume->setPt( pt );
#endif


    LOG(LEVEL) 
              << " lvIdx (aka meshIdx) " << lvIdx 
              << " ndIdx (aka volIdx) " << ndIdx 
              << " spec " << spec 
              ; 

    return volume ; 
}




GVolume* GMaker::makeBox(glm::vec4& param)
{
    float size = param.w ; 

    gfloat3 mn( param.x - size, param.y - size, param.z - size );
    gfloat3 mx( param.x + size, param.y + size, param.z + size );

    //gbbox bb(gfloat3(-size), gfloat3(size));  
    gbbox bb(mn, mx);  

    return makeBox(bb);
}


GVolume* GMaker::makeBox(gbbox& bbox)
{
    LOG(debug) << "GMaker::makeBox" ;

    unsigned int nvert = 24 ; 
    unsigned int nface = 6*2 ; 

    gfloat3* vertices = new gfloat3[nvert] ;
    guint3* faces = new guint3[nface] ;
    gfloat3* normals = new gfloat3[nvert] ;

    // TODO: migrate to NTrianglesNPY::cube ?
    GBBoxMesh::twentyfour(bbox, vertices, faces, normals );

    unsigned int meshindex = 0 ; 
    unsigned int nodeindex = 0 ; 

    GMesh* mesh = new GMesh(meshindex, vertices, nvert,  
                                       faces, nface,    
                                       normals,  
                                       NULL ); // texcoords

    mesh->setColors(  new gfloat3[nvert]);
    mesh->setColor(0.5,0.5,0.5);  


    // TODO: tranform hookup with NTrianglesNPY 
    GMatrixF* transform = new GMatrix<float>();

    GVolume* volume = new GVolume(nodeindex, transform, mesh, NULL, -1 );     

    volume->setBoundary(0);     // unlike ctor these create arrays


    return volume ; 
}


GVolume* GMaker::makePrism(glm::vec4& param, const char* spec)
{

    NTrianglesNPY* tris = NTrianglesNPY::prism(param);

    unsigned int meshindex = 0 ; 
    unsigned int nodeindex = 0 ; 

    GMesh* mesh = GMeshMaker::Make(tris->getTris(), meshindex);
    //mesh->dumpNormals("GMaker::makePrism normals", 24);

    glm::mat4 txf = tris->getTransform(); 
    GMatrixF* transform = new GMatrix<float>(glm::value_ptr(txf));

    GVolume* volume = new GVolume(nodeindex, transform, mesh, NULL, -1 );     
    volume->setBoundary(0);     // these setters create arrays
    nprism prism(param.x, param.y, param.z, param.w);
    npart  pprism = prism.part();

    GParts* pts = GParts::Make(pprism, spec);

    volume->setParts(pts);

    return volume ; 
}







GVolume* GMaker::makeSubdivSphere(glm::vec4& param, unsigned int nsubdiv, const char* type)
{
    LOG(debug) << "GMaker::makeSubdivSphere" 
              << " nsubdiv " << nsubdiv
              << " type " << type
              << " param " << gformat(param) 
              ;

    NTrianglesNPY* tris = makeSubdivSphere(nsubdiv, type);

    float radius = param.w ; 

    glm::vec3 scale(radius);
    glm::vec3 translate(param.x,param.y,param.z);  // formerly only z shifts were honoured
    tris->setTransform(scale, translate);   

    return makeSphere(tris);
}



NTrianglesNPY* GMaker::makeSubdivSphere(unsigned int nsubdiv, const char* type)
{
    // approach to using geodesic subdiv for partial spheres 
    // http://www.unitbv.ro/faculties/biblio/Buletin_univ/pdf/Iacob.pdf

    NTrianglesNPY* tris(NULL);
    if(strcmp(type,"I")==0)
    {
        unsigned int ntri = 20*(1 << (nsubdiv * 2)) ;
        NTrianglesNPY* icos = NTrianglesNPY::icosahedron();
        tris = icos->subdivide(nsubdiv);  // (subdiv, ntri)  (0,20) (3,1280)
        assert(tris->getNumTriangles() == ntri);
    }
    else if(strcmp(type,"O")==0)
    {
        unsigned int ntri = 8*(1 << (nsubdiv * 2)) ;
        NTrianglesNPY* octa = NTrianglesNPY::octahedron();
        tris = octa->subdivide(nsubdiv); 
        assert(tris->getNumTriangles() == ntri);
    }
    else if(strcmp(type,"HO")==0)
    {
        unsigned int ntri = 4*(1 << (nsubdiv * 2)) ;
        NTrianglesNPY* ho = NTrianglesNPY::hemi_octahedron(); 
        tris = ho->subdivide(nsubdiv); 
        assert(tris->getNumTriangles() == ntri);
    }
    else if(strcmp(type,"HOS")==0)
    {
        unsigned int ntri = 4*(1 << (nsubdiv * 2)) ;
        glm::vec3 tr(0.,0.,0.5);
        glm::vec3 sc(1.,1.,1.);
        glm::mat4 m = glm::scale(glm::translate(glm::mat4(1.0), tr), sc);
        NTrianglesNPY* ho = NTrianglesNPY::hemi_octahedron(); 
        NTrianglesNPY* tho = ho->transform(m);
        tris = tho->subdivide(nsubdiv); 
        assert(tris->getNumTriangles() == ntri);
    }
    else if(strcmp(type,"C")==0)
    {
        unsigned int ntri = 2*6*(1 << (nsubdiv * 2)) ;
        NTrianglesNPY* cube = NTrianglesNPY::cube();
        tris = cube->subdivide(nsubdiv); 
        assert(tris->getNumTriangles() == ntri);
    }
    assert(tris);
    return tris ; 
}


GVolume* GMaker::makeZSphere(glm::vec4& param)
{
    NTrianglesNPY* ll = NTrianglesNPY::sphere(param);
    NTrianglesNPY* dk = NTrianglesNPY::disk(param);
    ll->add(dk);
    return makeSphere(ll);
}


void GMaker::makeBooleanComposite(char shapecode, std::vector<GVolume*>& /*volumes*/,  glm::vec4& /*param*/, const char* /*spec*/)
{
    assert( shapecode == 'I' || shapecode == 'J' || shapecode == 'K' );

    // hmm rustling up a trianglulated boolean composite is real difficult to do in general, 
    // but tis just for viz so could use bbox placeholder ?
}

GVolume* GMaker::makeZSphereIntersect_DEAD(glm::vec4& param, const char* spec)
{
    assert(0 && "NOT WORKING, NO POINT FIXING : AS NCSG APPROACH SO MUCH BETTER SEE tlens-- " );

    // parameters of two spheres with offset z positions used
    // to create a lens shape composed of 2 back-to-back zspheres

    float a_radius = param.x ; 
    float b_radius = param.y ; 
    float a_zpos   = param.z ;
    float b_zpos   = param.w ; 

    LOG(info) << "GMaker::makeZSphereIntersect"
              << " a_radius " << a_radius 
              << " b_radius " << b_radius 
              << " a_zpos " << a_zpos
              << " b_zpos " << b_zpos
              ;


    assert(b_zpos > a_zpos); 

    /*                           
                 +------------+   B
          A     /              \
           +---/---+            \
          /   /     \            |
         /   /       \           |
        |    |       |           | 
        |    |       |           |

    */

    // TODO: such npy- level geo-specifics should
    //       be moved into npy- and handled at a higher
    //       from here

    nsphere* a = nsphere::Create(0,0,a_zpos,a_radius);
    nsphere* b = nsphere::Create(0,0,b_zpos,b_radius);
    ndisk* d = nsphere::intersect(a,b) ;   // from NPlane.hpp, not same as ndisc (degenerated ncylinder)
    float zd = d->z();

    d->dump("ndisk from nsphere::intersect" ); 
    LOG(info) << "ndisk::dump DONE " ; 


    // two CSG_SPHERE part
    npart ar = a->zrhs(d); 
    npart bl = b->zlhs(d);

    ar.dump("ar: a.zrhs(d) ");
    bl.dump("bl: b.zlhs(d) ");


    glm::vec3 a_center = a->center();
    glm::vec3 b_center = b->center();

    // ctmin, ctmax, zpos, radius
    glm::vec4 arhs_param( a->costheta(zd), 1.f, a_center.z, a->radius()) ;
    glm::vec4 blhs_param( -1, b->costheta(zd),  b_center.z, b->radius()) ;

    NTrianglesNPY* a_tris = NTrianglesNPY::sphere(arhs_param);
    NTrianglesNPY* b_tris = NTrianglesNPY::sphere(blhs_param);

    GVolume* a_volume = makeSphere(a_tris);
    GVolume* b_volume = makeSphere(b_tris);

    GParts* a_pts = GParts::Make(ar, spec);
    GParts* b_pts = GParts::Make(bl, spec);

    a_volume->setParts(a_pts);
    b_volume->setParts(b_pts);

    //volumes.push_back(a_volume);
    //volumes.push_back(b_volume);

    a_volume->addChild(b_volume);
    return a_volume ; 
}


GVolume* GMaker::makeSphere(NTrianglesNPY* tris)
{
    // TODO: generalize to makeSolid by finding other way to handle normals ?

    unsigned int meshindex = 0 ; 
    unsigned int nodeindex = 0 ; 

    NPY<float>* triangles = tris->getTris();

    glm::mat4 txf = tris->getTransform(); 

    GMesh* mesh = GMeshMaker::MakeSphereLocal(triangles, meshindex);

    GMatrixF* transform = new GMatrix<float>(glm::value_ptr(txf));

    //transform->Summary("GMaker::makeSphere");

    GVolume* volume = new GVolume(nodeindex, transform, mesh, NULL, -1 );     

    volume->setBoundary(0);     // these setters create arrays

    return volume ; 
}

