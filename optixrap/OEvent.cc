#include "SLog.hh"
#include "NPY.hpp"
#include "OpticksEvent.hh"

#include "OContext.hh"
#include "OEvent.hh"
#include "OBuf.hh"

#include "PLOG.hh"


OpticksEvent* OEvent::getEvent()
{
    return m_evt ; 
}
void OEvent::setEvent(OpticksEvent* evt)
{
    m_evt = evt ; 

}

// canonical single OEvent instance resides in OPropagator 
// and is instanciated with OPropagator

OEvent::OEvent(OContext* ocontext)
   :
   m_log(new SLog("OEvent::OEvent")),
   m_ocontext(ocontext),
   m_context(ocontext->getContext()),
   m_evt(NULL),
   m_genstep_buf(NULL),
   m_photon_buf(NULL),
   m_record_buf(NULL),
   m_sequence_buf(NULL),
   m_buffers_created(false)
{
    (*m_log)("DONE");
}


void OEvent::createBuffers(OpticksEvent* evt)
{
    // NB in INTEROP mode the OptiX buffers for the evt data 
    // are actually references to the OpenGL buffers created 
    // with createBufferFromGLBO by Scene::uploadEvt Scene::uploadSelection

    assert(m_buffers_created==false);
    m_buffers_created = true ; 
 
    NPY<float>* gensteps =  evt->getGenstepData() ;
    assert(gensteps);
    m_genstep_buffer = m_ocontext->createBuffer<float>( gensteps, "gensteps");
    m_context["genstep_buffer"]->set( m_genstep_buffer );
    m_genstep_buf = new OBuf("genstep", m_genstep_buffer);

    NPY<float>* photon = evt->getPhotonData() ; 
    assert(photon);
    m_photon_buffer = m_ocontext->createBuffer<float>( photon, "photon");
    m_context["photon_buffer"]->set( m_photon_buffer );
    m_photon_buf = new OBuf("photon", m_photon_buffer);

    NPY<short>* rx = evt->getRecordData() ;
    assert(rx);
    m_record_buffer = m_ocontext->createBuffer<short>( rx, "record");
    m_context["record_buffer"]->set( m_record_buffer );
    m_record_buf = new OBuf("record", m_record_buffer);

    NPY<unsigned long long>* sq = evt->getSequenceData() ;
    assert(sq);
    m_sequence_buffer = m_ocontext->createBuffer<unsigned long long>( sq, "sequence"); 
    m_context["sequence_buffer"]->set( m_sequence_buffer );
    m_sequence_buf = new OBuf("sequence", m_sequence_buffer);
    m_sequence_buf->setMultiplicity(1u);
    m_sequence_buf->setHexDump(true);

}

void OEvent::resizeBuffers(OpticksEvent* evt)
{
    NPY<float>* gensteps =  evt->getGenstepData() ;
    assert(gensteps);
    OContext::resizeBuffer<float>(m_genstep_buffer, gensteps, "gensteps");

    NPY<float>* photon = evt->getPhotonData() ; 
    assert(photon);
    OContext::resizeBuffer<float>(m_photon_buffer,  photon, "photon");

    NPY<short>* rx = evt->getRecordData() ; 
    assert(rx);
    OContext::resizeBuffer<short>(m_record_buffer,  rx, "record");

    NPY<unsigned long long>* sq = evt->getSequenceData() ; 
    assert(sq);
    OContext::resizeBuffer<unsigned long long>(m_sequence_buffer, sq , "sequence");
}



void OEvent::upload(OpticksEvent* evt)   
{
    LOG(info) << "OEvent::upload id " << evt->getId() ; 
    setEvent(evt);

    if(!m_buffers_created)
    {
        createBuffers(evt);
    }
    else
    {
        resizeBuffers(evt);
    }


    NPY<float>* gensteps =  evt->getGenstepData() ;

    if(m_ocontext->isCompute()) 
    {
        LOG(info) << "OEvent::upload (COMPUTE)" << " uploading gensteps " ;
        OContext::upload<float>(m_genstep_buffer, gensteps);
    }
    else if(m_ocontext->isInterop())
    {
        assert(gensteps->getBufferId() > 0); 
        LOG(info) << "OEvent::upload (INTEROP)" 
                  << " gensteps handed to OptiX by referencing OpenGL buffer id  "
                  ;
    }
    LOG(info) << "OEvent::upload DONE" ; 
}

void OEvent::download(unsigned mask)
{
    download(m_evt, mask);
}
void OEvent::download(OpticksEvent* evt, unsigned mask)
{
    assert(evt) ;
    LOG(info)<<"OEvent::download id " << evt->getId()  ;
 
    if(mask & GENSTEP)
    {
        NPY<float>* genstep = evt->getGenstepData();
        OContext::download<float>( m_genstep_buffer, genstep );
    }
    if(mask & PHOTON)
    {
       NPY<float>* photon = evt->getPhotonData();
       OContext::download<float>( m_photon_buffer, photon );
    }
    if(mask & RECORD)
    {
        NPY<short>* rx = evt->getRecordData();
        OContext::download<short>( m_record_buffer, rx );
    }
    if(mask & SEQUENCE)
    {
        NPY<unsigned long long>* sq = evt->getSequenceData();
        OContext::download<unsigned long long>( m_sequence_buffer, sq );
    }

    LOG(info)<<"OEvent::download DONE" ;
}



OBuf* OEvent::getSequenceBuf()
{
    return m_sequence_buf ; 
}
OBuf* OEvent::getPhotonBuf()
{
    return m_photon_buf ; 
}
OBuf* OEvent::getGenstepBuf()
{
    return m_genstep_buf ; 
}
OBuf* OEvent::getRecordBuf()
{
    return m_record_buf ; 
}

