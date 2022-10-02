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


#include <algorithm>
#include "X4Entity.hh"
#include "SLOG.hh"

X4Entity* X4Entity::fEntity = NULL ; 


X4Entity_t X4Entity::Type(const char* name)
{
    if(fEntity == NULL) fEntity = new X4Entity ; 
    return fEntity->type(name);
}

const char* X4Entity::Name(X4Entity_t type)
{
    if(fEntity == NULL) fEntity = new X4Entity ; 
    return fEntity->name(type);
}


X4Entity::X4Entity()
{
    VN& n = m_names ;  
    VT& t = m_types ; 

    // generated by x4-entity Mon Jun 11 19:35:59 HKT 2018 
    n.push_back("G4DisplacedSolid")     ; t.push_back(_G4DisplacedSolid)      ;
    n.push_back("G4UnionSolid")         ; t.push_back(_G4UnionSolid)          ;
    n.push_back("G4IntersectionSolid")  ; t.push_back(_G4IntersectionSolid)   ;
    n.push_back("G4SubtractionSolid")   ; t.push_back(_G4SubtractionSolid)    ;

    n.push_back("G4MultiUnion")         ; t.push_back(_G4MultiUnion)          ; 
    n.push_back("G4Box")                ; t.push_back(_G4Box)                 ; 
    n.push_back("G4Cons")               ; t.push_back(_G4Cons)                ; 
    n.push_back("G4EllipticalCone")     ; t.push_back(_G4EllipticalCone)      ; 
    n.push_back("G4Ellipsoid")          ; t.push_back(_G4Ellipsoid)           ; 
    n.push_back("G4EllipticalTube")     ; t.push_back(_G4EllipticalTube)      ; 
    n.push_back("G4ExtrudedSolid")      ; t.push_back(_G4ExtrudedSolid)       ; 
    n.push_back("G4Hype")               ; t.push_back(_G4Hype)                ; 
    n.push_back("G4Orb")                ; t.push_back(_G4Orb)                 ; 
    n.push_back("G4Para")               ; t.push_back(_G4Para)                ; 
    n.push_back("G4Paraboloid")         ; t.push_back(_G4Paraboloid)          ; 
    n.push_back("G4Polycone")           ; t.push_back(_G4Polycone)            ; 
    n.push_back("G4GenericPolycone")    ; t.push_back(_G4GenericPolycone)     ; 
    n.push_back("G4Polyhedra")          ; t.push_back(_G4Polyhedra)           ; 
    n.push_back("G4Sphere")             ; t.push_back(_G4Sphere)              ; 
    n.push_back("G4TessellatedSolid")   ; t.push_back(_G4TessellatedSolid)    ; 
    n.push_back("G4Tet")                ; t.push_back(_G4Tet)                 ; 
    n.push_back("G4Torus")              ; t.push_back(_G4Torus)               ; 
    n.push_back("G4GenericTrap")        ; t.push_back(_G4GenericTrap)         ; 
    n.push_back("G4Trap")               ; t.push_back(_G4Trap)                ; 
    n.push_back("G4Trd")                ; t.push_back(_G4Trd)                 ; 
    n.push_back("G4Tubs")               ; t.push_back(_G4Tubs)                ; 
    n.push_back("G4CutTubs")            ; t.push_back(_G4CutTubs)             ; 
    n.push_back("G4TwistedBox")         ; t.push_back(_G4TwistedBox)          ; 
    n.push_back("G4TwistedTrap")        ; t.push_back(_G4TwistedTrap)         ; 
    n.push_back("G4TwistedTrd")         ; t.push_back(_G4TwistedTrd)          ; 
    n.push_back("G4TwistedTubs")        ; t.push_back(_G4TwistedTubs)         ; 

}


X4Entity_t X4Entity::type( const char* name )
{
    VN& n = m_names ;  
    VT& t = m_types ; 

    VN::iterator it = std::find(n.begin(), n.end(), name ) ; 
    bool found = it != n.end() ; 

    LOG_IF(fatal, !found) << "invalid name " << name ; 

    assert( found ) ;  
    size_t idx = std::distance( n.begin(), it  ) ; 

    //LOG(info) << " idx " << idx ; 

    return t[idx] ; 
}

const char* X4Entity::name( X4Entity_t type )
{
    VN& n = m_names ;  
    VT& t = m_types ; 

    VT::iterator it = std::find(t.begin(), t.end(), type ) ; 
    bool found = it != t.end() ; 

    LOG_IF(fatal, !found) << "invalid type " << type ; 

    assert( found ) ;  
    size_t idx = std::distance( t.begin(), it  ) ; 

    //LOG(info) << " idx " << idx ; 

    return n[idx].c_str() ; 
}


