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

#include <sstream>

#include "G4Step.hh"
#include "G4StepPoint.hh"
#include "CStp.hh"
#include "CRecorder.hh"
#include "CAction.hh"
#include "OpticksPhoton.h"
#include "OpticksPhoton.hh"
#include "OpStatus.hh"

#include "Format.hh"


// ctor used for debug dumping of live recording 
#ifdef USE_CUSTOM_BOUNDARY
CStp::CStp(const G4Step* step, int step_id, Ds::DsG4OpBoundaryProcessStatus boundary_status, unsigned premat, unsigned postmat, unsigned preflag, unsigned postflag, CStage::CStage_t stage, int action, const G4ThreeVector& origin) 
#else
CStp::CStp(const G4Step* step, int step_id,   G4OpBoundaryProcessStatus boundary_status, unsigned premat, unsigned postmat, unsigned preflag, unsigned postflag, CStage::CStage_t stage, int action, const G4ThreeVector& origin) 
#endif
   :
   m_step(new G4Step(*step)), 
   m_step_id(step_id),
   m_boundary_status(boundary_status),
   m_premat(premat),
   m_postmat(postmat),
   m_preflag(preflag),
   m_postflag(postflag),
   m_stage(stage),
   m_action(action),
   m_origin(origin)
{
}


// ctor used for post recording : currently recommended for its clarity  
#ifdef USE_CUSTOM_BOUNDARY
CStp::CStp(const G4Step* step, int step_id, Ds::DsG4OpBoundaryProcessStatus boundary_status, CStage::CStage_t stage, const G4ThreeVector& origin) 
#else
CStp::CStp(const G4Step* step, int step_id,  G4OpBoundaryProcessStatus boundary_status, CStage::CStage_t stage, const G4ThreeVector& origin) 
#endif
   :
   m_step(new G4Step(*step)), 
   m_step_id(step_id),
   m_boundary_status(boundary_status),
   m_premat(0),
   m_postmat(0),
   m_preflag(0),
   m_postflag(0),
   m_stage(stage),
   m_action(0),
   m_origin(origin)
{
}


CStp::~CStp()
{
    delete m_step ; 
}




const G4Step* CStp::getStep() const 
{
   return m_step ; 
}
int CStp::getStepId() const 
{
   return m_step_id ; 
}
#ifdef USE_CUSTOM_BOUNDARY
Ds::DsG4OpBoundaryProcessStatus CStp::getBoundaryStatus() const 
#else
G4OpBoundaryProcessStatus   CStp::getBoundaryStatus() const 
#endif
{
   return m_boundary_status ;  
}
CStage::CStage_t CStp::getStage() const 
{
   return m_stage ; 
}




std::string CStp::origin() const 
{
    std::stringstream ss ; 
    ss 
       << ::Format(m_origin, "Ori", 4 ) 
       ;
    return ss.str(); 
}



// setters only used with post recording, not live recording 
void CStp::setMat(  unsigned premat, unsigned postmat)
{
    m_premat = premat ; 
    m_postmat = postmat ; 
}
void CStp::setFlag( unsigned preflag, unsigned postflag )
{
    m_preflag = preflag ; 
    m_postflag = postflag ; 
}
void CStp::setAction(int action)
{
    m_action = action ; 
}


std::string CStp::description_key() const 
{
    std::stringstream ss ; 
    ss 
       << " "   << "m_preflag/m_postflag"
       << "   " << "m_boundary_status"
       << "   " << std::setw(50) << "m_action"
       << std::endl
       << "["
       << std::setw(4) << "m_step_id"
       << "]"       
       << " ::Format(m_step,m_origin, \"Stp\" ) "
       ; 
    return ss.str(); 
}

std::string CStp::description() const 
{
    std::stringstream ss ; 
    ss 
       << " " << OpticksPhoton::Abbrev(m_preflag) << "/" << OpticksPhoton::Abbrev(m_postflag) 
       << "   " << std::setw(5) << OpStatus::OpBoundaryAbbrevString(m_boundary_status) 
       << "   " << std::setw(50) << CAction::Action(m_action) 
       << std::endl 
       << "[" 
       << std::setw(4) << m_step_id 
       << "]"       
       << ::Format(m_step, m_origin, "Stp" ) 
       ;
    return ss.str(); 
}

