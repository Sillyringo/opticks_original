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
#include <map>

// brap-
#include "BTimeKeeper.hh"

// npy-
#include "SeqNPY.hpp"
#include "PhotonsNPY.hpp"
#include "RecordsNPY.hpp"
#include "BoundariesNPY.hpp"
#include "SequenceNPY.hpp"
#include "G4StepNPY.hpp"
#include "Types.hpp"

#include "GMaterialLib.hh"
#include "GBndLib.hh"
#include "GItemIndex.hh"

#include "Opticks.hh"
#include "OpticksAttrSeq.hh"
#include "OpticksIdx.hh"
#include "OpticksHub.hh"
#include "OpticksRun.hh"
#include "OpticksEvent.hh"

#include "PLOG.hh"

const plog::Severity OpticksIdx::LEVEL = PLOG::EnvLevel("OpticksIdx", "debug"); 


/**
OpticksIdx
===========

Canonical instance is ctor resident of OKMgr or OKG4Mgr 

**/


OpticksIdx::OpticksIdx(OpticksHub* hub)
   :
   m_hub(hub), 
   m_ok(hub->getOpticks()),
   m_run(m_ok->getRun())
{
}

OpticksEvent* OpticksIdx::getEvent()
{
    OpticksEvent* evt = m_run->getCurrentEvent();
    return evt ; 
}

GItemIndex* OpticksIdx::makeHistoryItemIndex()
{
    OpticksEvent* evt = getEvent();
    Index* seqhis_ = evt->getHistoryIndex() ;
    if(!seqhis_)
    {
         LOG(error) << "OpticksIdx::makeHistoryItemIndex NULL seqhis" ;
         return NULL ; 
    }
 
    OpticksAttrSeq* qflg = m_hub->getFlagNames();

    if(LEVEL < debug) // see PLogTest for levels ints
    {
        qflg->dumpTable(seqhis_, "OpticksIdx::makeHistoryItemIndex seqhis"); 
    }


    GItemIndex* seqhis = new GItemIndex(seqhis_) ;  
    seqhis->setTitle("Photon Flag Sequence Selection");
    seqhis->setHandler(qflg);  // OpticksAttrSeq (attributed sequence) handler provides the labels
    seqhis->formTable();

    return seqhis ; 
}




OpticksAttrSeq* OpticksIdx::getMaterialNames()
{
     OpticksAttrSeq* qmat = m_hub->getMaterialLib()->getAttrNames();
     qmat->setCtrl(OpticksAttrSeq::SEQUENCE_DEFAULTS);
     return qmat ; 
}

OpticksAttrSeq* OpticksIdx::getBoundaryNames()
{
     GBndLib* blib = m_hub->getBndLib();
     OpticksAttrSeq* qbnd = blib->getAttrNames();
     if(!qbnd->hasSequence())
     {    
         blib->close();
         assert(qbnd->hasSequence());
     }    
     qbnd->setCtrl(OpticksAttrSeq::VALUE_DEFAULTS);
     return qbnd ;
}






GItemIndex* OpticksIdx::makeMaterialItemIndex()
{
    OpticksEvent* evt = getEvent();
    Index* seqmat_ = evt->getMaterialIndex() ;
    if(!seqmat_)
    {
         LOG(warning) << "OpticksIdx::makeMaterialItemIndex NULL seqmat" ;
         return NULL ; 
    }
 
    OpticksAttrSeq* qmat = getMaterialNames();

    GItemIndex* seqmat = new GItemIndex(seqmat_) ;  
    seqmat->setTitle("Photon Material Sequence Selection");
    seqmat->setHandler(qmat);
    seqmat->formTable();

    return seqmat ; 
}

GItemIndex* OpticksIdx::makeBoundaryItemIndex()
{
    OpticksEvent* evt = getEvent();
    Index* bndidx_ = evt->getBoundaryIndex();
    if(!bndidx_)
    {
         LOG(error) << "NULL bndidx from OpticksEvent" ;
         return NULL ; 
    }
 
    OpticksAttrSeq* qbnd = getBoundaryNames();
    //qbnd->dumpTable(bndidx, "OpticksIdx::makeBoundariesItemIndex bndidx"); 

    GItemIndex* boundaries = new GItemIndex(bndidx_) ;  
    boundaries->setTitle("Photon Termination Boundaries");
    boundaries->setHandler(qbnd);
    boundaries->formTable();

    return boundaries ; 
}
 
void OpticksIdx::indexSeqHost()
{
    LOG(info) << "OpticksIdx::indexSeqHost" ; 

    OpticksEvent* evt = getEvent();
    if(!evt) return ; 

    NPY<unsigned long long>* ph = evt->getSequenceData();

    if(ph && ph->hasData())
    {
        SeqNPY* seq = new SeqNPY(ph);
        seq->dump("OpticksIdx::indexSeqHost");
        std::vector<int> counts = seq->getCounts();
        delete seq ; 

        G4StepNPY* g4step = m_run->getG4Step();
        assert(g4step && "OpticksIdx::indexSeqHost requires G4StepNPY, created in translate"); 
        g4step->checkCounts(counts, "OpticksIdx::indexSeqHost checkCounts"); 
    }
    else
    { 
        LOG(warning) << "OpticksIdx::indexSeqHost requires sequence data hostside " ;      
    }
}







std::map<unsigned int, std::string> OpticksIdx::getBoundaryNamesMap()
{
    OpticksAttrSeq* qbnd = getBoundaryNames() ;
    return qbnd->getNamesMap(OpticksAttrSeq::ONEBASED) ;
}






void OpticksIdx::indexBoundariesHost()
{
    // Indexing the final signed integer boundary code (p.flags.i.x = prd.boundary) from optixrap-/cu/generate.cu
    // see also opop-/OpIndexer::indexBoundaries for GPU version of this indexing 
    // also see optickscore-/Indexer for another CPU version 

    OpticksEvent* evt = getEvent();
    if(!evt) return ; 

    NPY<float>* dpho = evt->getPhotonData();
    if(dpho && dpho->hasData())
    {
        // host based indexing of unique material codes, requires downloadEvt to pull back the photon data
        LOG(info) << "OpticksIdx::indexBoundaries host based " ;
        std::map<unsigned int, std::string> boundary_names = getBoundaryNamesMap();
        BoundariesNPY* bnd = new BoundariesNPY(dpho);
        bnd->setBoundaryNames(boundary_names);
        bnd->indexBoundaries();
        evt->setBoundariesNPY(bnd);
    }
    else
    {
        LOG(warning) << "OpticksIdx::indexBoundariesHost dpho NULL or no data " ;
    }

}



