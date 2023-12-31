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

#pragma once

#include <string>

// trying to fwd declare leads to linker errors for static NPY methods with some tests : G4StepNPYTest.cc, HitsNPYTest.cc see tests/CMakeLists.txt
//template <typename T> class NPY ; 
#include "NPY.hpp"
#include "plog/Severity.h"
#include "NGLM.hpp"
#include "NPY_API_EXPORT.hh"


struct NPY_API NGLMCF
{
    static const plog::Severity LEVEL ; 

    const glm::mat4& A ; 
    const glm::mat4& B ; 

    static constexpr float epsilon_translation = 1e-3 ; 
    static constexpr float epsilon = 1e-5 ; 

    float diff ; 
    float diff2 ; 
    float diffFractional ;
    glm::mat4 diffFractionalCheck ; 
    float diffFractionalMax ;
    float diffMax ;
    bool match ;
 
    NGLMCF( const glm::mat4& A_, const glm::mat4& B_ ) ;
    std::string desc(const char* msg="NGLMCF::desc", int width=12); 

};



template <typename T>
struct NPY_API NGLMCF_
{
    static const plog::Severity LEVEL ; 

    const glm::tmat4x4<T>& A ; 
    const glm::tmat4x4<T>& B ; 

    static constexpr T epsilon_translation = 1e-3; 
    static constexpr T epsilon  = 1e-5 ; 

    T diff ; 
    T diff2 ; 
    T diffFractional ;
    glm::tmat4x4<T> diffFractionalCheck ; 
    T diffFractionalMax ;
    T diffMax ;
    bool match ;
 
    NGLMCF_( const glm::tmat4x4<T>& A_, const glm::tmat4x4<T>& B_ ) ;
    std::string desc(const char* msg="NGLMCF_::desc", int width=12); 

};




