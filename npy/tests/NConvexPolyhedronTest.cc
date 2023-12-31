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

#include <iostream>
#include <iomanip>

#include "SSys.hh"
#include "NCSG.hpp"
#include "NConvexPolyhedron.hpp"
#include "NBBox.hpp"
#include "NGLMExt.hpp"
#include "GLMFormat.hpp"
#include "NSceneConfig.hpp"
#include "NNodePoints.hpp"

#include "OPTICKS_LOG.hh"



nconvexpolyhedron* test_load(const char* path)
{
    LOG(info) << "test_load " << path ;  

    const char* gltfconfig = "csg_bbox_parsurf=1" ;
    const NSceneConfig* config = new NSceneConfig(gltfconfig) ; 

    nnode* root = nnode::Load(path, config );

    assert(root->type == CSG_CONVEXPOLYHEDRON );
    nconvexpolyhedron* cpol = (nconvexpolyhedron*)root  ;
    cpol->verbosity = 1 ;  
    cpol->pdump("cpol->pdump");

    return cpol ; 
}

nconvexpolyhedron* test_make()
{
    nquad param ; 
    nquad param1 ; 
    nquad param2 ; 
    nquad param3 ; 

    param.u = {0,0,0,0} ;
    param1.u = {0,0,0,0} ;
    param2.u = {0,0,0,0} ;
    param3.u = {0,0,0,0} ;

    nconvexpolyhedron* cpol = nconvexpolyhedron::Create(param, param1, param2, param3 );
    return cpol ; 
}


void test_sdf(const nconvexpolyhedron* cpol)
{
    std::cout << "cpol(50,50,50) = " << (*cpol)(50,50,50) << std::endl ;
    std::cout << "cpol(-50,-50,-50) = " << (*cpol)(-50,-50,-50) << std::endl ;

    for(float v=-400.f ; v <= 400.f ; v+= 100.f )
    {
        float sd = (*cpol)(v,0,0) ;

        std::cout 
            << " x  " << std::setw(10) << v
            << " sd:  " << std::setw(10) << sd
/*
            << " y  " << std::setw(10) << (*cpol)(0,v,0)
            << " z  " << std::setw(10) << (*cpol)(0,0,v)
            << " xy " << std::setw(10) << (*cpol)(v,v,0)
            << " xz " << std::setw(10) << (*cpol)(v,0,v)
            << " yz " << std::setw(10) << (*cpol)(0,v,v)
            << "xyz " << std::setw(10) << (*cpol)(v,v,v)
*/
            << std::endl ; 
    } 
}



void test_intersect(const nconvexpolyhedron* cpol)
{
    glm::vec3 ray_origin(0,0,0);
    float t_min = 0.f ; 

    for(unsigned i=0 ; i < 3 ; i++)  
    {
        for(unsigned j=0 ; j < 2 ; j++)
        {
            glm::vec3 ray_direction(0,0,0);
            ray_direction.x = i == 0  ? (j == 0 ? 1 : -1 ) : 0 ; 
            ray_direction.y = i == 1  ? (j == 0 ? 1 : -1 ) : 0 ; 
            ray_direction.z = i == 2  ? (j == 0 ? 1 : -1 ) : 0 ; 
            std::cout << " dir " << ray_direction << std::endl ; 

            glm::vec4 isect(0.f);
            bool valid_intersect = cpol->intersect(  t_min , ray_origin, ray_direction , isect );
            assert(valid_intersect);

            std::cout << " isect : " << isect << std::endl ; 
        }
    }
}


void test_bbox(const nconvexpolyhedron* cpol)
{
    nbbox bb = cpol->bbox() ; 
    std::cout << bb.desc() << std::endl ; 
}


void test_getSurfacePointsAll(nconvexpolyhedron* cpol)
{
    cpol->dump_planes(); 

    unsigned level = 1 ;  // +---+---+
    int margin = 1 ;      // o---*---o
    unsigned verbosity = 1 ; 
    unsigned prim_idx = 0 ; 

    cpol->collectParPoints(prim_idx, level, margin, FRAME_LOCAL, verbosity);
    const std::vector<glm::vec3>& surf = cpol->par_points ; 

    LOG(info) << "test_parametric"
              << " surf points " << surf.size()
              ;

    for(unsigned i=0 ; i < surf.size() ; i++ ) std::cout << gpresent(surf[i]) << std::endl ; 

}



void test_dumpSurfacePointsAll(const nconvexpolyhedron* cpol)
{
    cpol->dumpSurfacePointsAll("dumpSurfacePointsAll", FRAME_LOCAL);
}


nconvexpolyhedron* test_make_trapezoid()
{
   /*
    z-order verts


                  6----------7
                 /|         /|
                / |        / |
               4----------5  |
               |  |       |  |                       
               |  |       |  |         Z    
               |  2-------|--3         |  Y
               | /        | /          | /
               |/         |/           |/
               0----------1            +------ X
                         

    x1: x length at -z
    y1: y length at -z

    x2: x length at +z
    y2: y length at +z

    z:  z length

    */

    LOG(info) << "test_make_trapezoid" ; 

    float z  = 200 ; 
    float x1 = 200 ; 
    float y1 = 200 ; 
    float x2 = 200 ; 
    float y2 = 200 ; 
  
    nconvexpolyhedron* cpol = nconvexpolyhedron::CreateTrapezoid( z,  x1,  y1,  x2,  y2 );

    cpol->dump_planes();
    cpol->dump_uv_basis();

    nbbox bb = cpol->bbox_model();
    std::cout << "bbox_model " << bb.desc() << std::endl ; 

    return cpol ; 
}


void test_collect_surface_points(nconvexpolyhedron* cpol )
{
    LOG(info) << "test_collect_surface_points" ; 

    cpol->verbosity = SSys::getenvint("VERBOSITY", 5) ; 

    // observe that low parsurf_level dont cover the whole
    // surface because meet target points count already ?

    NSceneConfig config("parsurf_level=4");

    NNodePoints pts(cpol, &config) ;  

    pts.collect_surface_points();
    pts.dump();
}




void test_transform_planes(nconvexpolyhedron* cpol )
{
    NPY<float>* planbuf = NPY<float>::make(cpol->planes);
    planbuf->dump("before");

    glm::mat4 placement = nglmext::make_translate(1002,-5000,10);

    nglmext::transform_planes(planbuf, placement );

    planbuf->dump("after");
}


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    //const char* path = argc > 1 ?  argv[1] : "$TMP/tboolean-trapezoid--/1" ;
    // nconvexpolyhedron*  cpol = test_load(path)
    //
    //test_sdf(cpol);
    //test_intersect(cpol);
    //test_bbox(cpol);
    //test_getSurfacePointsAll(cpol);
    //test_dumpSurfacePointsAll(cpol);

    nconvexpolyhedron* cpol = test_make_trapezoid();
    test_collect_surface_points(cpol);

    //test_transform_planes(cpol);


    return 0 ; 
}

