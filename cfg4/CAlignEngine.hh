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
#include <unordered_set>
#include "plog/Severity.h"
#include "CFG4_API_EXPORT.hh"
#include "CFG4_HEAD.hh"
template <typename T> class NPY ; 

/**
CAlignEngine
==============

* TODO: move all use of CAlignEngine to CRandomEngine ?
  which uses dynamuic TCURAND approach

Used by::

   CCerenkovGenerator::GeneratePhotonsFromGenstep  when ALIGN_DEBUG active 
   G4Opticks::setAlignIndex   



CAlignEngine isa CLHEP::HepRandomEngine, when enabled it 

independant streams of pre-cooked random numbers.
Enabling and disabling are done via the static method:: 

    CAlignEngine::SetSequenceIndex(int seq_idx)   

When seq_idx is 0 or more the CAlignEngine is set as "theEngine"
and G4UniformRand() will supply the precooked sequence corresponding 
to the seq_idx.  When seq_idx is negative the engine in place previously
is restored as theEngine and ordinary non-precooked random numbers
are returned from G4UniformRand().

Dev Notes
----------

CAlignEngine works by maintaining an integer cursor for each stream,
which gets incremented by flat calls.  This allows  
switching around between streams.

**/

#include <ostream>
#include "CLHEP/Random/RandomEngine.h"

class CFG4_API CAlignEngine : public CLHEP::HepRandomEngine 
{
        friend struct CAlignEngineTest ; 
    public:
        static const plog::Severity LEVEL ; 
        static const char* SEQ_PATH ; 
    public:
        static bool SeqPathExists(); 
        static bool Initialize(const char* ssdir); 
        static void Finalize(); 
        static void SetSequenceIndex(int record_id); 
        double flat() ;  
    private:
        static CAlignEngine* INSTANCE ; 
        static const char* LOGNAME ; 
        static const char* InitSimLog( const char* ssdir );
    private:
        CAlignEngine(const char* ssdir);
        virtual ~CAlignEngine(); 

        void setSequenceIndex(int record_id); 
        std::string desc() const ; 
        bool isReady() const ; 
    private:
        const char*              m_seq_path ; 
        NPY<double>*             m_seq ; 
        double*                  m_seq_values ; 
        int                      m_seq_ni ; 
        int                      m_seq_nv ; 
        NPY<int>*                m_cur ; 
        int*                     m_cur_values ; 
        int                      m_seq_index ; 
        bool                     m_recycle ;   // temporary measure to decide on how much needs to be precooked 
        CLHEP::HepRandomEngine*  m_default ; 
        const char*              m_sslogpath ; 
        bool                     m_backtrace ; 
        std::ostream*            m_out ; 
        int                      m_count ; 
        int                      m_modulo ; 
    private:
        bool isTheEngine() const ; 
        void enable() const ; 
        void disable() const ; 
    private:
        std::string name() const ;
        void flatArray (const int size, double* vect);
        void setSeed(long , int) ; 
        void setSeeds(const long * , int) ; 
        void saveStatus( const char * ) const ; 
        void restoreStatus( const char * ); 
        void showStatus() const ; 

        std::unordered_set<unsigned> m_recycle_idx ; 
};

#include "CFG4_TAIL.hh"

 
