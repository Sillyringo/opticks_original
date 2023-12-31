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

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <iomanip>

#include <boost/lexical_cast.hpp>

#include "SDigest.hh"
#include "GVector.hh"
#include "GOpticalSurface.hh"

#include "SLOG.hh"
// trace/debug/info/warning/error/fatal


const char* GOpticalSurface::getName() const
{
    return m_name ; 
}
const char* GOpticalSurface::getType() const
{
    return m_type ; 
}
const char* GOpticalSurface::getModel() const
{
    return m_model ; 
}
const char* GOpticalSurface::getFinish() const
{
    return m_finish ; 
}
int GOpticalSurface::getFinishInt() const 
{
    return std::atoi(m_finish); 
}

const char* GOpticalSurface::getValue() const 
{
    return m_value ; 
}
const char* GOpticalSurface::getShortName() const 
{
    return m_shortname ; 
}


/*
g4-cls G4OpticalSurface
 source/materials/include/G4OpticalSurface.hh 

 61 enum G4OpticalSurfaceFinish
 62 {
 63    polished,                    // smooth perfectly polished surface
 64    polishedfrontpainted,        // smooth top-layer (front) paint
 65    polishedbackpainted,         // same is 'polished' but with a back-paint
 66 
 67    ground,                      // rough surface
 68    groundfrontpainted,          // rough top-layer (front) paint
 69    groundbackpainted,           // same as 'ground' but with a back-paint
 70 

*/

const char* GOpticalSurface::polished_ = "polished" ;
const char* GOpticalSurface::polishedfrontpainted_ = "polishedfrontpainted" ;
const char* GOpticalSurface::polishedbackpainted_  = "polishedbackpainted" ;
const char* GOpticalSurface::ground_ = "ground" ;
const char* GOpticalSurface::groundfrontpainted_ = "groundfrontpainted" ;
const char* GOpticalSurface::groundbackpainted_  = "groundbackpainted" ;

const char* GOpticalSurface::Finish(unsigned finish)
{
    const char* s = NULL ;
    switch(finish)
    {
       case 0: s =  polished_             ; break ;
       case 1: s =  polishedfrontpainted_ ; break ;
       case 2: s =  polishedbackpainted_  ; break ;
       case 3: s =  ground_               ; break ;
       case 4: s =  groundfrontpainted_   ; break ;
       case 5: s =  groundbackpainted_    ; break ;
       default: assert(0 && "unexpected optical surface finish") ; break ;
    } 
    return s ;  
}


bool GOpticalSurface::IsPolished(unsigned finish) // static 
{
    return finish == 0 || finish == 1 || finish == 2 ; 
}
bool GOpticalSurface::IsGround(unsigned finish) // static 
{
    return finish == 3 || finish == 4 || finish == 5 ; 
}






/*
     65 enum G4SurfaceType
     66 {
     67    dielectric_metal,            // dielectric-metal interface
     68    dielectric_dielectric,       // dielectric-dielectric interface
     69    dielectric_LUT,              // dielectric-Look-Up-Table interface
     70    dielectric_dichroic,         // dichroic filter interface
     71    firsov,                      // for Firsov Process
     72    x_ray                        // for x-ray mirror process
     73 };

*/

const char* GOpticalSurface::dielectric_dielectric_ = "dielectric_dielectric" ;
const char* GOpticalSurface::dielectric_metal_      = "dielectric_metal" ;

const char* GOpticalSurface::Type(unsigned type)
{
    const char* s = NULL ;
    switch(type)
    {
       case 0: s =  dielectric_metal_      ; break ;
       case 1: s =  dielectric_dielectric_ ; break ;
       default: assert(0 && "unexpected optical surface type") ; break ;
    } 
    return s ;  
}

std::string GOpticalSurface::brief(const guint4& optical)
{
    std::stringstream ss ; 

    unsigned index  = optical.x ; 
    unsigned type   = optical.y ; 
    unsigned finish = optical.z ; 
    unsigned value  = optical.w ; 

    ss << optical.description()
       << " "
       << "(" << std::setw(3) << index << ") "
       << std::setw(30) << Type(type)
       << std::setw(30) << Finish(finish)
       << " value "  << value 
       ;

    return ss.str();
}
    
GOpticalSurface* GOpticalSurface::create(const char* name, guint4 optical )
{
    std::string type   = boost::lexical_cast<std::string>(optical.y);   
    std::string finish = boost::lexical_cast<std::string>(optical.z);   
    std::string model  = "1" ; // always unified so skipped?  was that 1?

    unsigned upercent = optical.w ; 
    float fraction = float(upercent)/100.f ; 
    std::string value = boost::lexical_cast<std::string>(fraction);
   
    return new GOpticalSurface( name, type.c_str(), model.c_str(), finish.c_str(), value.c_str() ); 
}


/**
GOpticalSurface::getOptical
------------------------------

+---------------+---------------------------+--------------------------+-------------------+   
| .x idx+1 ?    |  .y  type                 |  .z  finish              |  .w value_percent |
+===============+===========================+==========================+===================+
|               |                           |                          |                   |
|               | 0:dielectric_metal        | 0:polished               |                   |
|               | 1:dielectric_dielectric   | 1:polishedfrontpainted   |                   |
|               |                           | 3:ground                 |                   | 
|               |                           |                          |                   |
+---------------+---------------------------+--------------------------+-------------------+

**/


guint4 GOpticalSurface::getOptical() const 
{
   guint4 optical ; 
   optical.x = UINT_MAX ; //  place holder
   optical.y = boost::lexical_cast<unsigned int>(getType()); 
   optical.z = boost::lexical_cast<unsigned int>(getFinish()); 

   const char* value = getValue();
   float percent = boost::lexical_cast<float>(value)*100.f ;   // express as integer percentage 
   unsigned upercent = unsigned(percent) ;   // rounds down 

   optical.w = upercent ;

   return optical ; 
}


GOpticalSurface::GOpticalSurface(const char* name, const char* type, const char* model, const char* finish, const char* value) 
    : 
    m_name(strdup(name)), 
    m_type(strdup(type)), 
    m_model(strdup(model)), 
    m_finish(strdup(finish)), 
    m_value(strdup(value)),
    m_shortname(NULL)
{
    init();
}

GOpticalSurface::GOpticalSurface(GOpticalSurface* other)
   :
   m_name(strdup(other->getName())),
   m_type(strdup(other->getType())),
   m_model(strdup(other->getModel())),
   m_finish(strdup(other->getFinish())),
   m_value(strdup(other->getValue())),
   m_shortname(NULL)
{
    init();
} 

void GOpticalSurface::init()
{
    findShortName();
    checkValue();
}

/**
GOpticalSurface::findShortName
--------------------------------

dyb names start /dd/... which is translated to __dd__
so detect this and apply the shortening by effectively 
trimminng the prefix 
     
juno names do not have the prefix so make shortname
the same as the full one
    
have to have different treatment as juno has multiple names ending _opsurf
which otherwise get shortened to "opsurf" and tripup the digest checking

**/

void GOpticalSurface::findShortName(char marker)
{
    if(m_shortname) return ;

    // strrchr : returns pointer to last occurrence of the marker "_" in m_name 

    m_shortname = m_name[0] == marker ? strrchr(m_name, marker) + 1 : m_name ; 

    LOG(debug) << __func__
              << " name [" << m_name << "]" 
              << " shortname [" << m_shortname << "]" 
              ;
}

/**
GOpticalSurface::checkValue
-----------------------------

Value must be between 0 and 1 inclusive

**/

void GOpticalSurface::checkValue() const 
{
    float value = boost::lexical_cast<float>(m_value) ; 
    bool is_allowed = value >= 0.f && value <= 1.f ; 
    LOG_IF(fatal, !is_allowed) 
        << " value unexpected " 
        << " value " << value 
        << " m_value " << m_value 
        << " m_name " << m_name
        ;
    assert( is_allowed ); 
}


bool GOpticalSurface::isGround()   const { return IsGround(  getFinishInt()) ; } 
bool GOpticalSurface::isPolished() const { return IsPolished(getFinishInt()) ; } 

/**
GOpticalSurface::isSpecular
---------------------------

Now returns true for all three polished finishes : polished, polishedfrontpainted, polishedbackpainted
Opticks treats all these three finishes as a specular surface. 

**/
bool GOpticalSurface::isSpecular() const { return isPolished() ; }



/**
GOpticalSurface::isSpecularOld
--------------------------------

Old way constrains finish to those that have been compared with Geant4


**/

bool GOpticalSurface::isSpecularOld() const 
{
    if(strncmp(m_finish,"0",strlen(m_finish))==0)  return true ;
    if(strncmp(m_finish,"1",strlen(m_finish))==0)  return true ;  // used by JUNO.Mirror_opsurf m_finish 1
    if(strncmp(m_finish,"3",strlen(m_finish))==0)  return false ;

    LOG(info) << "GOpticalSurface::isSpecular " 
              << " m_shortname " << ( m_shortname ? m_shortname : "-" )
              << " m_finish "    << ( m_finish ? m_finish : "-" ) 
              ;
   
    assert(0 && "expecting m_finish to be 0:polished or 3:ground ");
    return false ; 
}



GOpticalSurface::~GOpticalSurface()
{
}


const char* GOpticalSurface::digest() const 
{
    SDigest dig ;
    dig.update_str( m_type );
    dig.update_str( m_model );
    dig.update_str( m_finish );
    dig.update_str( m_value );
    return dig.finalize();
}


void GOpticalSurface::Summary(const char* msg, unsigned int /*imod*/) const 
{
    printf("%s : type %s model %s finish %s value %4s shortname %s \n", msg, m_type, m_model, m_finish, m_value, m_shortname );
}

std::string GOpticalSurface::description() const 
{
    std::stringstream ss ; 
    ss << " GOpticalSurface " 
       << " type " << m_type 
       << " model " << m_model 
       << " finish " << m_finish
       << " finishInt " << getFinishInt()
       << " isPolished " << isPolished()
       << " isGround "   << isGround()
       << " isSpecular " << isSpecular()
       << " value " << std::setw(5) << m_value
       << std::setw(30) << m_shortname 
       ;

    return ss.str();
}

