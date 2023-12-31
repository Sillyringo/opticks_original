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
#include <cfloat>
#include "NGLMExt.hpp"
#include "nmat4triple.hpp"

#include "GLMFormat.hpp"
#include "NGenerator.hpp"
#include "NBox.hpp"
#include "NBBox.hpp"
#include "Nuv.hpp"

#include "OPTICKS_LOG.hh"


void test_gtransform()
{
    nbbox bb ; 
    bb.min = {-200.f, -200.f, -200.f };
    bb.max = { 200.f,  200.f,  200.f };

    NGenerator gen(bb);

    bool verbose = !!getenv("VERBOSE")  ; 
    glm::vec3 tlate ;

    for(int i=0 ; i < 100 ; i++)
    {
        gen(tlate); 

        glm::mat4 t = glm::translate(glm::mat4(1.0f), tlate );
        glm::mat4 v = nglmext::invert_tr(t);
        glm::mat4 q = glm::transpose(v); 

        //nmat4pair pair(tr, irit);
        nmat4triple triple(t, v, q);

        if(verbose)
        std::cout << " gtransform " << triple << std::endl ; 

        nbox* a = nbox::Create(0.f,0.f,0.f,100.f, CSG_BOX);      
        // untouched box at origin

        nbox* b = nbox::Create(0.f,0.f,0.f,100.f, CSG_BOX);      
        b->gtransform = &triple ;  
        // translated box via gtransform

        nbox* c = nbox::Create( tlate.x, tlate.y, tlate.z,100.f, CSG_BOX);  
        // manually positioned box at tlated position 


        float x = 0 ; 
        float y = 0 ; 
        float z = 0 ; 

        for(int iz=-200 ; iz <= 200 ; iz+= 10 ) 
        {
           z = iz ;  
           float a_ = (*a)(x,y,z) ;
           float b_ = (*b)(x,y,z) ;
           float c_ = (*c)(x,y,z) ;
      
           if(verbose) 
           std::cout 
                 << " z " << std::setw(10) << z 
                 << " a_ " << std::setw(10) << std::fixed << std::setprecision(2) << a_
                 << " b_ " << std::setw(10) << std::fixed << std::setprecision(2) << b_
                 << " c_ " << std::setw(10) << std::fixed << std::setprecision(2) << c_
                 << std::endl 
                 ; 

           assert( b_ == c_ );

        }
    }
}


void test_sdf()
{
    nbox* b = nbox::Create(0,0,0,1, CSG_BOX);  // unit box centered at origin       

    for(float x=-2.f ; x < 2.f ; x+=0.01f )
    {
        float sd1 = b->sdf1(x,0,0) ;
        float sd2 = b->sdf2(x,0,0) ;

       /*
        std::cout
            << " x " << std::setw(5) << x 
            << " sd1 " << std::setw(5) << sd1 
            << " sd2 " << std::setw(5) << sd2
            << std::endl ;  
        */

        assert(sd1 == sd2 );
    }

}


void test_parametric()
{
    LOG(info) << "test_parametric" ;

    nbox* box = nbox::Create(0,0,0,100, CSG_BOX); 

    unsigned nsurf = box->par_nsurf();
    assert(nsurf == 6);

    unsigned nu = 1 ; 
    unsigned nv = 1 ; 
    unsigned p = 0 ; 

    for(unsigned s=0 ; s < nsurf ; s++)
    {
        std::cout << " surf : " << s << std::endl ; 

        for(unsigned u=0 ; u <= nu ; u++){
        for(unsigned v=0 ; v <= nv ; v++)
        {
            nuv uv = make_uv(s,u,v,nu,nv, p );

            glm::vec3 p = box->par_pos_global(uv);

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



void test_box_box3()
{
    LOG(info) << "test_box_box3" ;


    nbox* box = nbox::Create(0,0,0,10, CSG_BOX);
    box->verbosity = 3 ;  
    box->pdump("nbox::Create(0,0,0,10, CSG_BOX)");

    nbox* box3 = nbox::Create(20,20,20, 0, CSG_BOX3);
    box3->verbosity = 3 ;  
    box3->pdump("nbox::Create(20,20,20,0, CSG_BOX3)");

}


nmat4triple* make_triple()
{
    std::string order = "srt" ;   //  <- translate is first, see NGLMExtTest 
    glm::vec3 tlat(0,0,500);
    glm::vec4 axis_angle(0,0,1,0);
    glm::vec3 scal(1,1,1);
    glm::mat4 transform = nglmext::make_transform(order, tlat, axis_angle, scal );

    return new nmat4triple(transform);
}


void test_nudge()
{
    LOG(info) << "test_nudge" ;

    unsigned level = 1 ; 
    int margin = 1 ;  

    float h = 10.f ;  
    nbox* box = nbox::Create(2*h,2*h,2*h,0, CSG_BOX3); 
    box->verbosity = 3 ;  

    nmat4triple* start = make_triple() ;
    box->gtransform = start ; 

    box->pdump("nbox::Create(2*h,2*h,2*h,0,CSG_BOX3)");


    unsigned prim_idx = 0 ;   
 
    box->collectParPoints( prim_idx, level, margin, FRAME_LOCAL, box->verbosity); 

    const std::vector<glm::vec3>& before = box->par_points ;

    assert(before.size() == 6 ); 

    if(start == NULL)
    { 
        assert( before[0].z == -h  );
        assert( before[1].z == h  );
        assert( before[2].x == -h  );
        assert( before[3].x == h  );
        assert( before[4].y == -h  );
        assert( before[5].y == h  );
    }

    //for(unsigned i=0 ; i < before.size() ; i++) std::cout << glm::to_string(before[i]) << std::endl ; 
  
    unsigned nudge_face = 1 ; // +Z nudge

    float delta = 1.f ; 
    box->nudge(nudge_face, delta);
    box->pdump("nbox::Create(2*h,2*h,2*h,0,CSG_BOX3) NUDGED");

    box->collectParPoints( prim_idx, level, margin, FRAME_LOCAL, box->verbosity ); 
    const std::vector<glm::vec3>& after = box->par_points ;

    assert(after.size() == 6 ); 

    if(start == NULL)
    {
        assert( after[0].z == -h );
        assert( after[1].z == h + delta );
        assert( after[2].x == -h  );
        assert( after[3].x == h  );
        assert( after[4].y == -h  );
        assert( after[5].y == h  );
    }
}


void test_getParPoints()
{
    float h = 10.f ;  
    nbox* box = nbox::Create(2*h,2*h,2*h,0,CSG_BOX3); 
    box->verbosity = 3 ;  
    box->pdump("nbox::Create(2*h,2*h,2*h,0,CSG_BOX3)");

    unsigned level = 1 ;  // +---+---+
    int margin = 1 ;      // o---*---o
    unsigned prim_idx = 0 ;   

    box->collectParPoints( prim_idx, level, margin, FRAME_LOCAL, box->verbosity ); 
    const std::vector<glm::vec3>& surf = box->par_points ; 

    LOG(info) << "test_getParPoints"
              << " surf " << surf.size()
              ;

    for(unsigned i=0 ; i < surf.size() ; i++ ) std::cout << gpresent(surf[i]) << std::endl ; 


    box->dumpSurfacePointsAll("box->dumpSurfacePointsAll", FRAME_LOCAL );


}





int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    //test_gtransform();
    //test_sdf();
    //test_parametric();
    //test_box_box3();
    //test_nudge();

    test_getParPoints();

    return 0 ; 
}




