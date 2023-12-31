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

/**
NTreePositive
=============

Canonical stack::

    X4PhysicalVolume::ConvertSolid
    X4PhysicalVolume::ConvertSolid_ 
    X4PhysicalVolume::ConvertSolid_FromRawNode
    NTreeProcess::init
    NTreePositive::init 

NTreePositive inplace positivizes the CSG tree of nodes, making 
the tree easier to rearrange as all the non-commutative
difference operators are replaced by corresponding 
commutative unions and intersections with 
complements applied to primitives.

The changes effected to the tree are 

1. some node types of operator nodes are changed
2. some complements are set on primitive nodes

* python equivalent ../analytic/csg.py 


Possible Issue : CSG_DIFFERENCE root node not positivized ?
--------------------------------------------------------------

Possibly JUNO geometry has no such G4VSolid so dont feel the issue.

**/

#include "NPY_API_EXPORT.hh"
#include "OpticksCSG.h"
#include "plog/Severity.h"

#include <vector>
#include <string>


template <typename T>
class NPY_API NTreePositive
{
        static const plog::Severity LEVEL ; 
    public:
        std::string desc() const ;
        NTreePositive(T* root); 
        T*    root() const ;
    private:
        void  init() ; 
        static void positivize_r(T* node, bool negate, unsigned depth);
    private:
        T*                     m_root ; 

};


