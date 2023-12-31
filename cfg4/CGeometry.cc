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

#include <string>
class OpticksQuery ; 

#include "BFile.hh"

#include "OpticksHub.hh"
#include "OpticksCfg.hh"
#include "Opticks.hh"

#include "NGLM.hpp"
#include "GLMFormat.hpp"

#include "CMaterialTable.hh"
#include "CMaterialBridge.hh"
#include "CSurfaceBridge.hh"
#include "CTestDetector.hh"
#include "CGDMLDetector.hh"

#include "CG4.hh"
#include "CDetector.hh"
#include "CGeometry.hh"
#include "CMaterialLib.hh"
#include "CSensitiveDetector.hh"

#include "PLOG.hh"

const plog::Severity CGeometry::LEVEL = PLOG::EnvLevel("CGeometry", "DEBUG") ; 



CMaterialLib*    CGeometry::getMaterialLib() const { return m_mlib ; }
CDetector*       CGeometry::getDetector() const { return m_detector; } 
CMaterialBridge* CGeometry::getMaterialBridge() const { return m_material_bridge ; }
CSurfaceBridge*  CGeometry::getSurfaceBridge() const { return m_surface_bridge ; }

const std::map<std::string, unsigned>& CGeometry::getMaterialMap() const 
{
    assert(m_material_table);
    return m_material_table->getMaterialMap();
}


/**
CGeometry::CGeometry
----------------------



**/

CGeometry::CGeometry(OpticksHub* hub, CSensitiveDetector* sd) 
    :
    m_hub(hub),
    m_sd(sd),
    m_ok(m_hub->getOpticks()),
    m_cfg(m_ok->getCfg()),
    m_detector(NULL),
    m_mlib(NULL),
    m_material_table(NULL),
    m_material_bridge(NULL),
    m_surface_bridge(NULL)
{
    init();
}

void CGeometry::init()
{
    CDetector* detector = NULL ; 
    if(m_ok->isTest())  // --test  : branch between CTestDetector and CGDMLDetector
    {
        LOG(LEVEL) << "G4 simple test geometry " ; 
        OpticksQuery* query = NULL ;  // normally no OPTICKS_QUERY geometry subselection with test geometries
        detector  = static_cast<CDetector*>(new CTestDetector(m_hub, query, m_sd)) ; 
    }
    else
    {
        // no options here: will load the .gdml sidecar of the geocache .dae 
        LOG(LEVEL) << "G4 GDML geometry " ; 
        OpticksQuery* query = m_ok->getQuery();
        detector  = static_cast<CDetector*>(new CGDMLDetector(m_hub, query, m_sd)) ; 
    }

    // detector->attachSurfaces();  moved into the ::init of CTestDetector and CGDMLDetector to avoid omission

    m_detector = detector ; 
    m_mlib = detector->getMaterialLib();
}


bool CGeometry::hookup(CG4* g4)
{
    g4->setUserInitialization(m_detector);


    glm::vec4 ce = m_detector->getCenterExtent();
    LOG(LEVEL) 
        << " center_extent " << gformat(ce) 
        ;    

    m_ok->setSpaceDomain(ce); // triggers Opticks::configureDomains

    return true ; 
}


/**
CGeometry::postinitialize
--------------------------

Invoked from CG4::postinitialize



**/

void CGeometry::postinitialize()
{
    // both these are deferred til here as needs G4 materials table to have been constructed 

    m_material_table = new CMaterialTable(); 
    //m_material_table->dump("CGeometry::postinitialize");

    GMaterialLib* mlib = m_hub->getMaterialLib(); 
    m_material_bridge = new CMaterialBridge( mlib ); 

    GSurfaceLib* slib = m_hub->getSurfaceLib(); 
    m_surface_bridge = new CSurfaceBridge( slib ); 

    // was surprised to find that CMaterialLib is that comes from detector is not 
    // converted as standard the materoal converts are called individually 

    CMaterialLib* clib = m_mlib ; 
    assert( clib );
    clib->postinitialize();

    export_();
}

void CGeometry::export_()
{
    bool expo = m_cfg->hasOpt("export");
    if(!expo) return ; 
    //std::string expodir = m_cfg->getExportConfig();
    
    const char* expodir = "$TMP/CGeometry" ;

    if(BFile::ExistsDir(expodir))
    {   
        BFile::RemoveDir(expodir); 
        LOG(info) << "CGeometry::export_ removed " << expodir ; 
    }   

    BFile::CreateDir(expodir);
    m_detector->export_dae(expodir, "CGeometry.dae");
    m_detector->export_gdml(expodir, "CGeometry.gdml");
}

