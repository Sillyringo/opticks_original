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

#include "SLOG.hh"

#include "GLMFormat.hpp"
#include "NGLMExt.hpp"
#include "nmat4triple.hpp"
#include <glm/gtx/component_wise.hpp>


#include "NBBox.hpp"
#include "NBox.hpp"
#include "NPart.hpp"
#include "NPlane.hpp"
#include "Nuv.hpp"

#include <cmath>
#include <cassert>
#include <cstring>

#include "OpticksCSG.h"




nbox* nbox::Create(const nquad& p, OpticksCSG_t type  ) // static
{
    assert( type == CSG_BOX3 || type == CSG_BOX ); 
    nbox* n = new nbox ; 
    nnode::Init(n,type) ; 
    n->param = p ; 
    n->_bbox_model = new nbbox(n->bbox_model()) ;   // bbox_model() has no transforms applied, so is available early
    return n ;
}


nbox* nbox::Create(float x, float y, float z, float w, OpticksCSG_t type  )  // static
{
    nquad param ;
    param.f =  {x,y,z,w} ;

    if( type == CSG_BOX3 )
    {
        assert( w == 0.f ) ;   // w is vestigial for CSG_BOX3, used by code gen
    }
    else if( type == CSG_BOX )
    {
        assert( w > 0.f ); 
    }

    return Create( param, type ); 
}







/**
~/opticks_refs/Procedural_Modelling_with_Signed_Distance_Functions_Thesis.pdf

SDF from point px,py,pz to box at origin with side lengths (sx,sy,sz) at the origin 

    max( abs(px) - sx/2, abs(py) - sy/2, abs(pz) - sz/2 )

**/




float nbox::operator()(float x_, float y_, float z_) const 
{
    glm::vec3 pos(x_,y_,z_);
    return sdf_(pos, gtransform);
} 

float nbox::sdf_model(const glm::vec3& pos) const { return sdf_(pos, NULL) ; }
float nbox::sdf_local(const glm::vec3& pos) const { return sdf_(pos, transform) ; }
float nbox::sdf_global(const glm::vec3& pos) const { return sdf_(pos, gtransform) ; }

float nbox::sdf_(const glm::vec3& pos, NNodeFrameType fty) const 
{
    float sd = 0.f ; 
    switch(fty)
    {
       case FRAME_MODEL  : sd = sdf_(pos, NULL)      ; break ; 
       case FRAME_LOCAL  : sd = sdf_(pos, transform) ; break ; 
       case FRAME_GLOBAL : sd = sdf_(pos, gtransform) ; break ; 
    }
    return sd ; 
}


float nbox::sdf_local_(const glm::vec3& pos, const glm::vec3& halfside)
{
    glm::vec3 a = glm::abs(pos) ;
    glm::vec3 d = a - halfside ; 
    float sd = glm::compMax(d) ;
    return sd ; 
}


/**
https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm

https://www.youtube.com/watch?v=62-pRVZuS5c

float sdBox( vec3 p, vec3 b )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
}

**/

float nbox::sdf_(const glm::vec3& pos, const nmat4triple* triple) const 
{
    glm::vec4 q(pos,1.0); 

    if(triple) q = triple->v * q ;  // NB using inverse transform on the query point 

    glm::vec3 c = center();

    glm::vec3 p = glm::vec3(q) - c ;  // coordinates in frame with origin at box center 

    glm::vec3 a = glm::abs(p) ;

    glm::vec3 h = halfside();

    glm::vec3 d = a - h ; 

    float sd = glm::compMax(d) ;

    return complement ? -sd : sd ; 
}

float nbox::sdf1(float x_, float y_, float z_) const 
{
    return (*this)(x_,y_,z_);
}

float nbox::sdf2(float x_, float y_, float z_) const 
{
    glm::vec4 p(x_,y_,z_,1.0); 

    if(gtransform) p = gtransform->v * p ;  

    glm::vec3 bmx = bmax() ; 

    // abs query point folds 3d space into +ve octant
    // subtract bmx places box max at origin in this folded space  

    glm::vec3 d = glm::abs(glm::vec3(p)) - bmx  ;

    float dmaxcomp = glm::compMax(d);

    glm::vec3 dmax = glm::max( d, glm::vec3(0.f) );

    float d_inside = fminf(dmaxcomp, 0.f);
    float d_outside = glm::length( dmax );

    return d_inside + d_outside ;       

   // see tests/NBoxTest.cc   sdf2 and sdf1 match despite code appearances
}

/*
http://iquilezles.org/www/articles/distfunctions/distfunctions.htm
float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}
*/



glm::vec3 nbox::bmax() const 
{
    glm::vec3 h = halfside(); 
    glm::vec3 c = center();    
    if(is_box3()) assert( c.x == 0 && c.y == 0 && c.z == 0 );
    return c + h ; 
}

glm::vec3 nbox::bmin() const 
{
    glm::vec3 h = halfside(); 
    glm::vec3 c = center();    
    if(is_box3()) assert( c.x == 0 && c.y == 0 && c.z == 0 );
    return c - h ; 
}

nbbox nbox::bbox_model() const
{
    glm::vec3 mi = bmin() ; 
    glm::vec3 mx = bmax() ; 

    nbbox bb = make_bbox(mi, mx, complement) ;

    return bb ; 
}

bool nbox::is_box() const { return type == CSG_BOX ;  }
bool nbox::is_box3() const { return type == CSG_BOX3 ;  }









unsigned nbox::par_nsurf() const 
{
   return 6 ; 
}
int nbox::par_euler() const 
{
   return 2 ; 
}
unsigned  nbox::par_nvertices(unsigned nu, unsigned nv) const 
{
   assert( nu >=1 && nv >= 1 ); 

/*

                 +-----------+
                /|          /| 
               / |         / |
              7-----8-----9  |
              |  |        |  |
              |  |        |  |
              4  +--5-----6--+
              | /         | /
              |/          |/
              1-----2-----3
 
                               (nu = 2, nv = 2)    


                (nu+1)*(nv+1) = 3*3 = 9      distinct vertices for one face

            (nu+1-2)*(nv+1-2) = 
                (nu-1)*(nv-1) = 1*1 = 1       mid-vertices (not shared)

                     2*(nv-1) = 2*1 = 2       u edges (shared with one other, so only count half of em)
                     2*(nu-1) = 2*1 = 2       v edges (ditto)

       
     fvert = lambda nu,nv:(nu-1)*(nv-1) + (nv-1) + (nu-1) 
     nvert = lambda nu,nv:8+6*fvert(nu,nv)

*/

   unsigned mid_vert = (nv+1-2)*(nu+1-2) ;
   unsigned edge_vert = (nv+1-2) + (nu+1-2) ;
   unsigned face_vert = mid_vert + edge_vert   ;  
   unsigned distinct_vert = 8 + 6*face_vert ; 

   return distinct_vert ; 
}



glm::vec3 nbox::par_pos_model( const nuv& uv) const 
{
    unsigned s = uv.s() ; 
    assert(s < par_nsurf());

    float fu = uv.fu() ;
    float fv = uv.fv() ;

    glm::vec3 pos ; 
    glm::vec3 nrm ; 
    par_posnrm_model(pos, nrm, s, fu, fv  );

/*
    std::cout << "nbox::par_pos_model"
              << " uv " << glm::to_string(uv) 
              << " pos " << glm::to_string(pos)
              << " nrm " << glm::to_string(nrm)
              << std::endl 
               ; 
*/

    return pos ; 
}



void nbox::par_posnrm_model( glm::vec3& p, glm::vec3& n, unsigned s, float fu, float fv ) const 
{ 
    /*

                 6-----------7
                /|          /| 
               / |         / |
              4-----------5  |
              |  |        |  |
              |  |        |  |
              |  2--------|--3
              | /         | /
              |/          |/
              0-----------1
          
        z
         |  y
         | /
         |/
         +---> x
        

    0:    (0,2,1) (1,2,3)  :   -z
    1:    (4,5,6) (6,5,7)  :   +z

    2:    (0,4,2) (2,4,6)  :   -x
    3:    (1,3,5) (5,3,7)  :   +x

    4:    (1,5,0) (0,5,4)  :   -y
    5:    (6,7,2) (2,7,3)  :   +y

    */

    //nbbox bb = bbox() ;  // NB bbox() has gtransform->t is already applied
    //nbbox bb = bbox_model() ;
 

    //   1 - uv[0] 
    //
    //      attempts to arrange mappings
    //      as view the cube from the 6 different
    //      directions to yield consistent face winding  
    //      
    //            (x,y) -> (u,v)
    //            (y,z) -> (u,v)
    //            (x,z) -> (u,v)
    // 


    const nbbox& bb = *_bbox_model ;   // from init_box

    p.x = 0.f ; 
    p.y = 0.f ; 
    p.z = 0.f ; 

    n.x = 0.f ; 
    n.y = 0.f ; 
    n.z = 0.f ; 


 
    switch(s)
    {
        case 0:{    // -Z
                  n.z = -1.f ; 
                  p.x = glm::mix( bb.min.x, bb.max.x, 1 - fu ) ;
                  p.y = glm::mix( bb.min.y, bb.max.y, fv ) ;
                  p.z = bb.min.z ;
               } 
               ; break ;
        case 1:{   // +Z
                  n.z = 1.f ; 
                  p.x = glm::mix( bb.min.x, bb.max.x, fu ) ;
                  p.y = glm::mix( bb.min.y, bb.max.y, fv ) ;
                  p.z = bb.max.z ;
               }
               ; break ;


        case 2:{   // -X
                  n.x = -1.f ; 
                  p.x = bb.min.x ;
                  p.y = glm::mix( bb.min.y, bb.max.y, 1 - fu ) ;
                  p.z = glm::mix( bb.min.z, bb.max.z, fv ) ;
               } 
               ; break ;
        case 3:{   // +X
                  n.x =  1.f ; 
                  p.x = bb.max.x ;
                  p.y = glm::mix( bb.min.y, bb.max.y, fu ) ;
                  p.z = glm::mix( bb.min.z, bb.max.z, fv ) ;
               }
               ; break ;
 


        case 4:{  // -Y
                  n.y = -1.f ; 
                  p.x = glm::mix( bb.min.x, bb.max.x, fu ) ;
                  p.y = bb.min.y ;
                  p.z = glm::mix( bb.min.z, bb.max.z, fv ) ;
               } 
               ; break ;
        case 5:{  // +Y
                  n.y = 1.f ; 
                  p.x = glm::mix( bb.min.x, bb.max.x, 1 - fu ) ;
                  p.y = bb.max.y ;
                  p.z = glm::mix( bb.min.z, bb.max.z, fv ) ;
               }
               ; break ;
    }

    
}


/**
nbox::nudge
-------------

How to nudge to avoid a coincidence ? 

* want to grow the coincident face by some delta eg ~2*epsilon 

* but CSG_BOX3 is positioned at 0,0,0 in model frame with 3 dimensions 
  ... so need to grow eg +Z face to avoid coincidence, but that 
  would require a compensating translation transform ???

* CSG_BOX is model frame placed but is symmetric, 
  hmm could non-uniformly scale OR rather just not support as CSG_BOX is only used in
  testing not real geometry.

* Does it matter that grow both +Z and -Z by a few epsilon, when only +Z is coincident ?  
  Currently envisage such nudges being applied only to subtracted 
  sub-objects, eg B in (A - B) or B in A*!B 

  YES, it does matter, consider (A - B) cutting a groove...
  growing out into empty space in +Z for the subtracted box B
  is needed to avoid the edge speckles and does not change 
  the geometry, (because there is no interesction between this growth and A)

  Whereas growing B in -Z would deepen the groove. 

             _______        +Z
       +-----+ . . +-----+   ^
       |     |  B  |     |   |  
       |  A  +-----+     |
       |                 |
       +-----------------+


* DO I NEED A SEPARATE nudge_transform BECAUSE HOW MUCH TO DELTA TRANSLATE 
  TO PREVENT MOVEMENT OF THE FIXED FACE DEPENDS ON CURRENT TRANSFORM
  (IF ITS NOT JUST A TRANSLATION WHICH COMMUTES)

  * this is mixed frame thinking... just think within the single primitive
    frame, just want to grow the box in one direction... if you compensate to 
    fix the face in the prim frame, it will be fixed in the composite one 

  * gtransforms are normally created for all primitives 
    when NCSG::import_r recursion gets down to them based on the
    heirarchy of transforms collected from the ancestors of the primitive in 
    the tree ... so that means need to change transform and then update gtransforms

**/

void nbox::nudge(unsigned s, float delta)
{
    assert( s < par_nsurf());
    assert( is_box3() && "nbox::nudge only implemented for CSG_BOX3 not CSG_BOX " );

    if(verbosity > 0)
    {
        std::cout << "nbox::nudge START" 
                  << " s " << s 
                  << " delta " << delta
                  << " param.f " << param.f.desc()
                  << " verbosity " << verbosity
                  << std::endl ;
    }

    glm::vec3 tlate(0,0,0) ; 

    // -Z :  grows in +-Z,  tlate in -Z keeping +Z in same position
    // +Z :  grows in +-Z,  tlate in +Z keeping -Z in same positiom 

    switch(s)
    {
        case 0:{ param.f.z += delta ; tlate.z = -delta/2.f ; } ; break ; // -Z
        case 1:{ param.f.z += delta ; tlate.z =  delta/2.f ; } ; break ; // +Z

        case 2:{ param.f.x += delta ; tlate.x = -delta/2.f ; } ; break ; // -X
        case 3:{ param.f.x += delta ; tlate.x =  delta/2.f ; } ; break ; // +X

        case 4:{ param.f.y += delta ; tlate.y = -delta/2.f ; } ; break ; // -Y 
        case 5:{ param.f.y += delta ; tlate.y =  delta/2.f ; } ; break ; // +Y 
    }

    // HMM MAYBE PREFERABLE TO NON-UNIFORMLY SCALE TO ACHIEVE A DELTA
    // RATHER THAN ACTUALLY CHANGING GEOMETRY, AS THAT CAN BE DONE TO ANYTHING ???

    if(verbosity > 0)
    std::cout << "nbox::nudge DONE " 
              << " s " << s 
              << " delta " << delta
              << " param.f " << param.f.desc()
              << " tlate " << glm::to_string(tlate)
              << std::endl ;


    if(transform == NULL) transform = nmat4triple::make_identity() ; 

    glm::mat4 compensate_ = nglmext::make_translate(tlate); 
    nmat4triple compensate(compensate_);

    bool reverse = true ;   // <-- think of the nudge as an origin frame inside the leaf 
    std::vector<const nmat4triple*> triples ; 
    triples.push_back(&compensate); 
    triples.push_back(transform); 
    const nmat4triple* compensated = nmat4triple::product(triples, reverse );

    // cannot use make_translated as need the translation first ...
    // transform = transform->make_translated(tlate, reverse, "nbox::nudge" ); 

    if(verbosity > 0)
    std::cout << "nbox::nudge"
              << std::endl 
              << gpresent("compensate_", compensate_ )
              << std::endl 
              << " changing primitive transform "
              << std::endl
              << gpresent("transform->t", transform->t) 
              << std::endl
              << " with "
              << std::endl
              << gpresent("compensate.t", compensate.t)
              << std::endl
              << " -> "
              << gpresent("compensated->t", compensated->t)
              << std::endl
              ;

    transform = compensated ; 
    gtransform = global_transform();  // product of transforms from heirarchy


}



/**
nbox::resizeToFit
------------------

Replaces existing dimensons with those of the argument bounding box 
with scale and delta applied.

**/

void nbox::resizeToFit(const nbbox& bb, float scale, float delta )
{
    if(is_box3())
    {
        // NB box3 always centered, see NBox2Test
        param.f.x = scale*(fabs(bb.min.x) + fabs(bb.max.x)) + delta ; 
        param.f.y = scale*(fabs(bb.min.y) + fabs(bb.max.y)) + delta ; 
        param.f.z = scale*(fabs(bb.min.z) + fabs(bb.max.z)) + delta ; 
        param.f.w = 0.f ; 
    }
    else
    {
        param.f = bb.center_extent() ; 
        param.f.w *= scale ; 
        param.f.w += delta ; 
    }
}


nbbox nbox::bbox_(NNodeFrameType fr) const 
{
    nbbox bb = bbox_model();
    nbbox tbb(bb);

    if(fr == FRAME_LOCAL && transform)
    {
        tbb = bb.make_transformed(transform->t);
    }
    else if(fr == FRAME_GLOBAL && gtransform)
    {
        tbb = bb.make_transformed(gtransform->t);
    }
    return tbb ; 
}

nbbox nbox::bbox_(const nmat4triple* triple) const
{
    nbbox bb = bbox_model();
    return triple ? bb.make_transformed(triple->t) : bb ; 
    // bbox transforms need TR not IR*IT as they apply directly to geometry 
    // unlike transforming the SDF point or ray tracing ray which needs the inverse irit 
}


nbbox nbox::bbox()        const { return bbox_(FRAME_GLOBAL) ; } 
nbbox nbox::bbox_global() const { return bbox_(FRAME_GLOBAL) ; } 
nbbox nbox::bbox_local()  const { return bbox_(FRAME_LOCAL) ; } 




glm::vec3 nbox::gseedcenter() const 
{
    return gtransform == NULL ? center() : glm::vec3( gtransform->t * glm::vec4(center(), 1.f ) ) ;
}

void nbox::pdump(const char* msg) const 
{
    std::cout 
              << std::setw(10) << msg 
              << " nbox::pdump "
              << " label " << ( label ? label : "-" )
              << " center " << center() 
              << " param.f " << param.f.desc() 
              << " side " << param.f.w 
              << " gseedcenter " << gseedcenter()
              << " transform? " << !!transform
              << " gtransform? " << !!gtransform
              << " is_box3 " << is_box3()
              << std::endl ; 


    if(verbosity > 1 && transform) 
         std::cout 
              << std::endl
              << gpresent("tr->t", transform->t) 
              << std::endl
              ;

    if(verbosity > 1 && gtransform) 
         std::cout 
              << std::endl
              << gpresent("gtr->t", gtransform->t) 
              << std::endl
              ;

/*
    if(verbosity > 2)
    {
        dumpSurfacePointsAll("nbox::pdump", FRAME_MODEL );
        dumpSurfacePointsAll("nbox::pdump", FRAME_LOCAL );
        dumpSurfacePointsAll("nbox::pdump", FRAME_GLOBAL );
    }
*/
}


bool nbox::is_equal( const nbox& other ) const 
{
    return 
          param.f.x == other.param.f.x &&
          param.f.y == other.param.f.y &&
          param.f.z == other.param.f.z &&
          param.f.w == other.param.f.w ;
}



float nbox::r1() const { return 0.f ; }
float nbox::z1() const { return 0.f ; }
float nbox::r2() const { return 0.f ; }
float nbox::z2() const { return 0.f ; } 




glm::vec3 nbox::center() const { return glm::vec3(x(), y(), z()) ; }

float nbox::x() const { return is_box() ? param.f.x : 0.f ; }
float nbox::y() const { return is_box() ? param.f.y : 0.f ; }
float nbox::z() const { return is_box() ? param.f.z : 0.f ; }
bool  nbox::is_centered() const { return x() == 0.f && y() == 0.f && z() == 0.f ; }
void  nbox::set_centered() 
{ 
    assert( is_box()) ; 
    param.f.x = 0.f ; 
    param.f.y = 0.f ; 
    param.f.z = 0.f ; 
}

glm::vec3 nbox::halfside() const 
{ 
    glm::vec3 h ; 
    if(type == CSG_BOX3)
    { 
        h.x = param.f.x/2.f ;
        h.y = param.f.y/2.f ;
        h.z = param.f.z/2.f ;
    }
    else if(type == CSG_BOX )
    {
        h.x = param.f.w ;
        h.y = param.f.w ;
        h.z = param.f.w ;
    }
    else
    {
        assert(0);
    }
    return h ;
}





