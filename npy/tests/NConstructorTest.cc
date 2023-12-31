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

#include "NOctools.hpp"

#include "NField3.hpp"
#include "NGrid3.hpp"
#include "NFieldGrid3.hpp"

#include "NBox.hpp"
#include "NBBox.hpp"
#include "NOct.hpp"


#include "OPTICKS_LOG.hh"


//template class NConstructor<NOct> ;


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);



    int nominal = 5 ; 
    int coarse  = 4 ; 
    int verbosity = 11 ; 


    nbox obj = make_nbox(0,0,0,7) ; 
    nbbox obb = obj.bbox() ;
    std::function<float(float,float,float)> fn = obj.sdf();


    glm::vec3 obb_min(obb.min.x, obb.min.y, obb.min.z);
    glm::vec3 obb_max(obb.max.x, obb.max.y, obb.max.z);

    NField<glm::vec3,glm::ivec3,3> field( &fn , obb_min, obb_max );
    LOG(info) << field.desc() ; 

    NGrid<glm::vec3,glm::ivec3,3> grid(nominal);
    LOG(info) << grid.desc() ; 

    NFieldGrid3<glm::vec3,glm::ivec3> fg(&field, &grid);



    nvec4     bbce = obb.center_extent();

    int nominal_size = 1 << grid.level  ; 

    float ijkExtent = nominal_size/2 ;      // eg 64.f
    float xyzExtent = bbce.w  ;
    float ijk2xyz = xyzExtent/ijkExtent ;     // octree -> real world coordinates

    nvec4 ce = make_nvec4(bbce.x, bbce.y, bbce.z, ijk2xyz );


    NConstructor<NOct>* ctor = new NConstructor<NOct>(&fg, ce, obb, nominal, coarse, verbosity );

    ctor->dump();


    return 0 ;   
}  
