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

#include <cstdlib>
#include "NGLMExt.hpp"

#include "NZSphere.hpp"
#include "NPart.hpp"
#include "NBBox.hpp"
#include "Nuv.hpp"

#include "OPTICKS_LOG.hh"




void test_dumpSurfacePointsAll()
{
    LOG(info) << "test_dumpSurfacePointsAll" ;
    nzsphere* zs = nzsphere::Create();
    zs->dumpSurfacePointsAll("zs.dumpSurfacePointsAll", FRAME_LOCAL);
}


void test_part()
{
    nzsphere* s = nzsphere::Create(0,0,0,10,-5,5);
    npart p = s->part();
    p.dump("p");
}

void test_bbox()
{
    nzsphere* a = nzsphere::Create(0.f,0.f,0.f,100.f, -50.f, 50.f);
    a->dump("zsph");

    nbbox bb = a->bbox();
    bb.dump("zbb");
}

void test_sdf()
{
    float radius = 10.f ; 
    float zdelta_min = -radius/2.f ; 
    float zdelta_max = radius/2.f ; 

    nzsphere* _a = nzsphere::Create(0.f,0.f,0.f,radius,zdelta_min,zdelta_max);
    const nzsphere& a = *_a ; 

    for(float v=-2*radius ; v <= 2*radius ; v+= radius/10.f ) 
        std::cout 
             << " v   " << std::setw(10) << v 
             << " x   " << std::setw(10) << std::fixed << std::setprecision(2) << a(v,0,0)
             << " y   " << std::setw(10) << std::fixed << std::setprecision(2) << a(0,v,0)
             << " z   " << std::setw(10) << std::fixed << std::setprecision(2) << a(0,0,v)
             << " xy  " << std::setw(10) << std::fixed << std::setprecision(2) << a(v,v,0)
             << " xz  " << std::setw(10) << std::fixed << std::setprecision(2) << a(v,0,v)
             << " yz  " << std::setw(10) << std::fixed << std::setprecision(2) << a(0,v,v)
             << " xyz " << std::setw(10) << std::fixed << std::setprecision(2) << a(v,v,v)
             << std::endl 
             ; 
}






void test_parametric()
{
    LOG(info) << "test_parametric" ;


    float radius = 10.f ; 
    float z1 = -radius/2.f ; 
    float z2 = radius/2.f ; 

    nzsphere* zs = nzsphere::Create(0.f,0.f,0.f,radius,z1,z2);



    unsigned nsurf = zs->par_nsurf();
    assert(nsurf == 3);

    unsigned nu = 5 ; 
    unsigned nv = 5 ; 
    unsigned prim_idx = 0 ; 

    for(unsigned s=0 ; s < nsurf ; s++)
    {
        std::cout << " surf : " << s << std::endl ; 

        for(unsigned u=0 ; u <= nu ; u++){
        for(unsigned v=0 ; v <= nv ; v++)
        {
            nuv uv = make_uv(s,u,v,nu,nv, prim_idx);

            glm::vec3 p = zs->par_pos_model(uv);

            std::cout 
                 << " s " << std::setw(3) << s  
                 << " u " << std::setw(3) << u  
                 << " v " << std::setw(3) << v
                 << " p " << glm::to_string(p)
                 << std::endl ;   
        }
        }
    }
}


void test_deltaTheta(float radius, float z1, float z2)
{
    nzsphere* zs = nzsphere::Create(0.f,0.f,0.f,radius,z1,z2 );
     
    LOG(info) << "test_deltaTheta" 
             << " radius " << radius
             << " z1 " << z1
             << " z2 " << z2
             << " startTheta " << zs->startTheta()  
             << " endTheta " << zs->endTheta()  
             << " deltaTheta " << zs->deltaTheta()  
             ;


}

void test_deltaTheta()
{
    float radius = 10.f ; 
    float z1 = -radius/2.f ; 
    float z2 = radius/2.f ; 

    test_deltaTheta(radius, z1, z2) ; 
    test_deltaTheta(500, -200, 200) ; 
}



int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    //test_part();
    //test_bbox();
    //test_sdf();

    //test_parametric();
    //test_dumpSurfacePointsAll();

    test_deltaTheta() ;

    return 0 ; 
}




