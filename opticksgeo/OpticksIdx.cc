
#include <string>
#include <map>

#include "SeqNPY.hpp"
#include "PhotonsNPY.hpp"
#include "HitsNPY.hpp"
#include "RecordsNPY.hpp"
#include "BoundariesNPY.hpp"
#include "SequenceNPY.hpp"
#include "G4StepNPY.hpp"
#include "Types.hpp"
#include "Timer.hpp"

#include "GGeo.hh"
#include "GItemIndex.hh"

#include "Opticks.hh"
#include "OpticksIdx.hh"
#include "OpticksHub.hh"
#include "OpticksRun.hh"
#include "OpticksEvent.hh"

#include "PLOG.hh"

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
         LOG(warning) << "OpticksIdx::makeHistoryItemIndex NULL seqhis" ;
         return NULL ; 
    }
 
    OpticksAttrSeq* qflg = m_hub->getFlagNames();
    //qflg->dumpTable(seqhis, "OpticksIdx::makeHistoryItemIndex seqhis"); 

    GItemIndex* seqhis = new GItemIndex(seqhis_) ;  
    seqhis->setTitle("Photon Flag Sequence Selection");
    seqhis->setHandler(qflg);
    seqhis->formTable();

    return seqhis ; 
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
 
    OpticksAttrSeq* qmat = m_hub->getMaterialNames();

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
         LOG(warning) << "OpticksIdx::makeBoundaryItemIndex NULL bndidx" ;
         return NULL ; 
    }
 
    OpticksAttrSeq* qbnd = m_hub->getBoundaryNames();
    //qbnd->dumpTable(bndidx, "OpticksIdx::makeBoundariesItemIndex bndidx"); 

    GItemIndex* boundaries = new GItemIndex(bndidx_) ;  
    boundaries->setTitle("Photon Termination Boundaries");
    boundaries->setHandler(qbnd);
    boundaries->formTable();

    return boundaries ; 
}
 


void OpticksIdx::indexEvtOld()
{
    OpticksEvent* evt = getEvent();
    if(!evt) return ; 

    // TODO: wean this off use of Types, for the new way (GFlags..)
    Types* types = m_ok->getTypes();
    Typ* typ = m_ok->getTyp();

    NPY<float>* ox = evt->getPhotonData();

    if(ox && ox->hasData())
    {
        PhotonsNPY* pho = new PhotonsNPY(ox);   // a detailed photon/record dumper : looks good for photon level debug 
        pho->setTypes(types);
        pho->setTyp(typ);
        evt->setPhotonsNPY(pho);

        GGeo* ggeo = m_hub->getGGeo();

        if(!ggeo) LOG(fatal) << "OpticksIdx::indexEvtOld" 
                             << " MUST OpticksHub::loadGeometry before OpticksIdx::indexEvtOld "
                             ;

        assert(ggeo);
        HitsNPY* hit = new HitsNPY(ox, ggeo->getSensorList());
        evt->setHitsNPY(hit);
    }

    NPY<short>* rx = evt->getRecordData();

    if(rx && rx->hasData())
    {
        RecordsNPY* rec = new RecordsNPY(rx, evt->getMaxRec(), evt->isFlat());
        rec->setTypes(types);
        rec->setTyp(typ);
        rec->setDomains(evt->getFDomain()) ;

        PhotonsNPY* pho = evt->getPhotonsNPY();
        if(pho)
        {
            pho->setRecs(rec);
        }
        evt->setRecordsNPY(rec);
    }

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
        std::map<unsigned int, std::string> boundary_names = m_hub->getBoundaryNamesMap();
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



