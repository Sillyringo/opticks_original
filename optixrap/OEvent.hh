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

#include "OXPPNS.hh"
#include "OpticksSwitches.h"
#include "plog/Severity.h"

class SLog ; 
class Opticks; 
class OpticksEvent ; 
class OContext ; 
class OBuf ; 

template <typename T> class NPY ; 

/**
OEvent
=======

OptiX buffers representing an OpticksEvent propagation.

The OEvent name is a bit misleading because there is 
only ever one OEvent instance.

The canonical single OEvent instance resides 
in OpEngine and is instanciated with OpEngine.
A pointer is also available in OPropagator, which
is also instanciated with OpEngine.

Buffers are created at the first *upload* and
are subsequently resized to correspond to the OpticksEvent. 

NOTE: This turns out to be problematic as the context is then not 
valid until the first upload. TODO: split creation of buffers from 
resize/population.


NB upload/download will only act on compute buffers, interop
buffers are skipped within underlying OContext methods
based on OpticksBufferControl settings.


::

    opticks-findl OEvent.hh

    ./okop/OpSeeder.cc
    ./okop/OpEngine.cc
    ./okop/OpIndexerApp.cc
    ./okop/OpZeroer.cc
    ./okop/OpIndexer.cc
    ./okop/tests/OpSeederTest.cc

    ./optixrap/OPropagator.cc
    ./optixrap/OEvent.cc

    ./optixrap/CMakeLists.txt
    ./optixrap/tests/OEventTest.cc
    ./optixrap/oxrap.bash

    ./ok/tests/VizTest.cc



Necessary Buffers
------------------

*genstep*
    (n_genstep,6,4) float32, parameters of Cerenkov, Scintillation or Torch genstep

*photon*
    (n_photon,4,4) float32

*seed*
    (n_photon, 1) uint32, provides genstep_id for each photon  


Buffers During Debugging
-------------------------

*sequence*
    (n_photon, 1, 2) uint64 (unsigned long long) : flag and material sequence (64 bits = 16*4 bits )

*record*
    (n_photon, 16, 2, 4) int16 (shorts) : highly domain compressed photon step records 

*boundary*
    (n_photon, 1, 4)  unsigned : uint4 packing of 16 signed char boundary indices, aka bndseq




**/

#include "OXRAP_API_EXPORT.hh"
class OXRAP_API OEvent 
{
    public:
        static const plog::Severity LEVEL ;  
    public:
        enum {
            GENSTEP  = 0x1 << 1, 
            PHOTON   = 0x1 << 2, 
            RECORD   = 0x1 << 3, 
            SEQUENCE = 0x1 << 4,
            SEED     = 0x1 << 5,
            SOURCE   = 0x1 << 6,
            DEBUG    = 0x1 << 7,
            WAY      = 0x1 << 8,
            BOUNDARY = 0x1 << 9
            };
    public:
        OEvent(Opticks* ok, OContext* ocontext);
        unsigned upload();
        unsigned download();
        void     downloadPhotonData();
        unsigned downloadHits();
        void     markDirty();
    private:
        unsigned upload(OpticksEvent* evt);
        unsigned uploadGensteps(OpticksEvent* evt);
#ifdef WITH_SOURCE
        unsigned uploadSource(OpticksEvent* evt);
#endif
        unsigned downloadHitsCompute(OpticksEvent* evt);
        unsigned downloadHitsInterop(OpticksEvent* evt);
    public:
        unsigned downloadHiys();
    private:
        unsigned downloadHiysCompute(OpticksEvent* evt);
        unsigned downloadHiysInterop(OpticksEvent* evt);
    public:
        OContext*     getOContext() const ;
        OpticksEvent* getEvent() const ;
        OBuf*         getSeedBuf() const ;
        OBuf*         getPhotonBuf() const ;
#ifdef WITH_SOURCE
        OBuf*         getSourceBuf() const ;
#endif
        OBuf*         getGenstepBuf() const ;
#ifdef WITH_RECORD
        OBuf*         getSequenceBuf() const ;
        OBuf*         getRecordBuf() const ;
#endif
    private:
        void init(); 
        unsigned initDownloadMask() const ;
        void createBuffers(OpticksEvent* evt);
        void resizeBuffers(OpticksEvent* evt);
        void setEvent(OpticksEvent* evt);
        void download(OpticksEvent* evt, unsigned mask);
    private:
        SLog*           m_log ; 
        Opticks*        m_ok ; 
        bool            m_way_enabled ; 
        unsigned        m_downloadmask ; 
        unsigned        m_hitmask ;  
        bool            m_compute ;  
        bool            m_dbghit ; 
        bool            m_dbgdownload ; 
        NPY<unsigned>*  m_mask ; 
        OContext*       m_ocontext ; 
        optix::Context  m_context ; 
        OpticksEvent*   m_evt ; 
        bool            m_photonMarkDirty ; 
#ifdef WITH_SOURCE
        bool            m_sourceMarkDirty ; 
#endif
        bool            m_seedMarkDirty ; 
    protected:
        optix::Buffer   m_genstep_buffer ; 
        optix::Buffer   m_photon_buffer ; 
#ifdef WITH_DEBUG_BUFFER
        optix::Buffer   m_debug_buffer ; 
#endif
        optix::Buffer   m_way_buffer ; 



#ifdef WITH_SOURCE
        optix::Buffer   m_source_buffer ; 
#endif
#ifdef WITH_RECORD
        optix::Buffer   m_record_buffer ; 
        optix::Buffer   m_sequence_buffer ; 
        optix::Buffer   m_boundary_buffer ; 
#endif
        optix::Buffer   m_seed_buffer ; 
    private:
        OBuf*           m_genstep_buf ;
        OBuf*           m_photon_buf ;
#ifdef WITH_SOURCE
        OBuf*           m_source_buf ;
#endif
#ifdef WITH_RECORD
        OBuf*           m_record_buf ;
        OBuf*           m_sequence_buf ;
        OBuf*           m_boundary_buf ;
#endif
        OBuf*           m_seed_buf ;

#ifdef WITH_DEBUG_BUFFER
        OBuf*           m_debug_buf ; 
#endif
        OBuf*           m_way_buf ; 

    private:
        bool            m_buffers_created ; 

};


