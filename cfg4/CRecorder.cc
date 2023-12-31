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

#include "G4SystemOfUnits.hh"
#include "CFG4_BODY.hh"

// okc-
#include "Opticks.hh"
#include "OpticksEvent.hh"
#include "OpticksFlags.hh"


#include "OpStatus.hh"

#include "CCtx.hh"
#include "CMaterialBridge.hh"
#include "CRec.hh"

#include "CPoi.hh"
#include "CStp.hh"

#include "CStep.hh"
#include "CAction.hh"

#include "CBoundaryProcess.hh"
#include "CRecorder.h"

#include "CDebug.hh"
#include "CWriter.hh"
#include "CRecorder.hh"
#include "Format.hh"


#include "PLOG.hh"


const plog::Severity CRecorder::LEVEL = PLOG::EnvLevel("CRecorder", "DEBUG") ; 

const char* CRecorder::PRE  = "PRE" ; 
const char* CRecorder::POST = "POST" ; 

std::string CRecorder::getStepActionString()
{
    return CAction::Action(m_state._step_action) ;
}

CRec* CRecorder::getCRec() const 
{
    return m_crec ;  
}

unsigned long long CRecorder::getSeqHis() const 
{
    return m_photon._seqhis ; 
}
unsigned long long CRecorder::getSeqMat() const 
{
    return m_photon._seqmat ; 
}

/**
dynamic:false is for when gensteps are available ahead of time

Q: But doesnt BeginOfGenstep signalling mean that always know the number of photons 
   for each genstep before getting its tracks  ? 

A: Probably dynamic:false means you have all gensteps for the event ahead of time 
   (not just one by one) so memory allocations can be done once only for the event. 


Q: how does the bookkeeping work for cerenkov photons that undergo reemission ?
A: 

**/

CRecorder::CRecorder(CCtx& ctx) 
    :
    m_ctx(ctx),
    m_ok(m_ctx.getOpticks()),
    m_microStep_mm(0.004),              //  see notes/issues/ok_lacks_SI-4BT-SD.rst
    m_suppress_same_material_microStep(true), 
    m_suppress_all_microStep(true), 
    m_mode(m_ok->getManagerMode()),   // --managermode
    m_recpoi(m_ok->isRecPoi()),   // --recpoi
    m_reccf(m_ok->isRecCf()),     // --reccf
    m_state(m_ctx),
    m_photon(m_ctx, m_state),

    m_crec(new CRec(m_ctx, m_state)),
    m_dbg(m_ctx.is_dbg() ? new CDebug(m_ctx, m_photon, this) : NULL),

    m_evt(NULL),
    m_material_bridge(NULL),
    m_live(false),
    m_writer(new CWriter(m_ctx, m_photon)),
    m_not_done_count(0)
{   
    LOG(LEVEL) << brief() ;
}


std::string CRecorder::brief() const 
{
    std::stringstream ss ; 
    ss 
       << " m_recpoi " << m_recpoi
       << " m_reccf " << m_reccf
       << " m_dbg " << ( m_dbg ? "Y" : "N" ) 
       ;
   return ss.str();
}


/**
CRecorder::postinitialize
----------------------------

Gets CMaterialBridge:m_material_bridge from CGeometry:m_geometry and passes this to m_crec


void CRecorder::postinitialize()
{
    CMaterialBridge* material_bridge = m_geometry->getMaterialBridge();
    setMaterialBridge(material_bridge); 
}
**/


void CRecorder::setMaterialBridge(const CMaterialBridge* material_bridge)
{
    assert( material_bridge ); 
    m_material_bridge = material_bridge ; 
    m_crec->setMaterialBridge( m_material_bridge );
    if(m_dbg) m_dbg->setMaterialBridge( m_material_bridge );
}



/**
CRecorder::initEvent
----------------------

Invoked by CManager::initEvent, configures and prepares for recording.

**/


void CRecorder::initEvent(OpticksEvent* evt)
{
    LOG(LEVEL); 
    assert(evt);
    m_writer->initEvent(evt);
    m_crec->initEvent(evt);
}


/**
CRecorder::BeginOfGenstep
----------------------------

When have input photons this is invoked by CManager::BeginOfEventAction
otherwise called by CManager::BeginOfGenstep

**/
void CRecorder::BeginOfGenstep()
{
    m_writer->BeginOfGenstep(); 
}


/**
CRecorder::postTrack
------------------------

Invoked by CTrackingAction::PostUserTrackingAction/CManager::PostUserTrackingAction

--recpoi
     not the default, has some truncation differences with --recstp  
     in principal it is more efficent : as it makes decision at collection 

--recstp 
     default
     unavoidably stores loads of StepTooSmall steps which are subsequently chucked
   
NB all this machinery is for validation purposes only, so should
not be concerned about operating efficiently.
 
**/

void CRecorder::postTrack() 
{
    LOG(LEVEL); 

    //m_ok->accumulateStart(m_postTrack_acc);  

    assert(!m_live);

    if(m_ctx._dbgrec) LOG(LEVEL) << "CRecorder::postTrack" ; 

    if(m_recpoi)  // --recpoi 
    {
        postTrackWritePoints();  
        if(m_reccf) compareModes() ; 
    }
    else        // default: --recstp
    {
        postTrackWriteSteps();
    } 

    if(m_dbg) m_dbg->postTrack(); 


    if(m_ctx._dump) // --dindex
    {
        LOG(LEVEL) << "[--dindex] " 
                  << " ctx " << m_ctx.brief()  
                  << " pho " << m_photon.brief() 
                  ; 
    }
    //m_ok->accumulateStop(m_postTrack_acc);  
}


/**
CRecorder::compareModes
----------------------------

Invoked from CRecorder::postTrack when using --reccf.
Asserts that the seqhis and seqmat obtained by --recpoi and --recstp modes match.

**/

void CRecorder::compareModes()
{
    CPhoton pp(m_photon); 

    m_writer->setEnabled(false);

    m_photon.clear();
    m_state.clear(); 
    postTrackWriteSteps();

    m_writer->setEnabled(true);

    CPhoton ps(m_photon); 

    if(ps._seqhis != pp._seqhis)
    {
        LOG(info) << m_ctx.desc() ; 
        LOG(info) << "ps:" << ps.desc() ; 
        LOG(info) << "pp:" << pp.desc() ;  
    }  

    assert( ps._seqhis == pp._seqhis );
    assert( ps._seqmat == pp._seqmat );
}

/**
CRecorder::Record
--------------------

Invoked by CManager::setStep for steps of optical G4Track after CCtx::setStep/CCtx::setStepOptical
This returns the "done" bool, which when true causes the track to be killed
enabling truncation behaviour to be matched between simulations. 

* recording and the writing are split

Not-zeroing m_slot for REJOINders 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* see notes/issues/reemission_review.rst


RE-joining : reunites reemission secondaries with their parents across all RE-generations
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Rejoining happens on output side not in the crec CStp list.

The rejoins of AB(actually RE) tracks with reborn secondaries 
are done by writing two (or more) sequences of track steps  
into the same record_id in the record buffer 
at the **appropriate non-zeroed slot**.

Notice that CWriter writes onto the photon record_id in the output arrays 
for REjoined tracks so from the point of view of record_id there is 
no requirement for reemission secondaries to be handled in sequence. 
BUT, as described below there is such a requirement for rejoin slots to be correct.

Q: How is the slot obtained for RE-join adding to the records ? 
   Can imagine it could be done by looking at the seqhis but do not find this code. 
   Makes me think that RE-joined step points will be overwriting : the lower slots ?
   But that would be very obvious. 

A: Explanation is that because CRecorder::Record does not call zeroPhoton for REJOIN 
   the slot stays elevated and the step records and photon history nibbles get written 
   into the appropriate slots.
   This means that contrary to some other notes there REALLY IS AN IMPLICIT ASSUMPTION:

   *  **RE-EMISSION SECONDARY G4Track MUST BE SUPPLIED TO CManager/CRecorder IN CORRECT GENERATIONAL SEQUENCE**.

   It seems that this sequential assumption must be being satisfied otherwise the step records 
   should be messed up rather obviously. 
   The sequential assumption is only for the continuation of the slots, not for the record_id which is random write
   into the dynamically growing buffer.

TODO: 

1. take a careful look at the RE steps, to make sure of the above answer
2. avoid the sequential assumption by extracting the slot-so-far from 
   the the seqhis history, eg with SBit::count_nibbles

**/

#ifdef USE_CUSTOM_BOUNDARY
bool CRecorder::Record(Ds::DsG4OpBoundaryProcessStatus boundary_status)
#else
bool CRecorder::Record(G4OpBoundaryProcessStatus boundary_status)
#endif
{    
    m_state._step_action = 0 ; 

    assert(!m_live);

    if(m_ctx._stage == CStage::START)  // first step of primary photon 
    { 
        zeroPhoton();
    }
    else if(m_ctx._stage == CStage::REJOIN ) // first step of RE-join photon
    {
        m_crec->clear(); 
        // NB Not-zeroing m_slot for REJOINders, see above notes
        // **CAUTION : SLOT CONTINUATION BY NOT-ZEROING IS RELYING ON THE SEQUENTIAL RE-EMISSION TRACK ASSUMPTION**
    }
    else if(m_ctx._stage == CStage::RECOLL )
    {
        m_state._decrement_request = 0 ;  
    } 

    bool done = m_crec->add(boundary_status) ; // collecting steps (recstp) or points (recpoi)

    if(m_ctx._dbgrec)
        LOG(info) 
            << "crec.add "
            << " boundary_status " << boundary_status
            << m_crec->desc()
            << std::setw(10) << CStage::Label(m_ctx._stage)
            << " " << m_ctx.desc_step() 
            << " " << ( done ? "DONE" : "-" )
            ; 

    return done ;  // (*lldb*) record
}

/**
CRecorder::zeroPhoton
-----------------------

Invoked from CRecorder::Record at the first step of primary photons,
clears: m_crec, m_photon, m_state

**/

void CRecorder::zeroPhoton()
{ 
    LOG(LEVEL) << m_photon.desc() ; 

    const G4StepPoint* pre = m_ctx._step->GetPreStepPoint() ;
    const G4ThreeVector& pos = pre->GetPosition();

    m_crec->setOrigin(pos);   // hmm maybe in CCtx already ?
    m_crec->clear();

    m_photon.clear();
    m_state.clear();

    if(m_dbg) m_dbg->Clear();
}




/**
CRecorder::postTrackWritePoints
----------------------------------

When using --recpoi mode this is invoked from CRecorder::postTrack 

**/


void CRecorder::postTrackWritePoints()
{ 
#ifdef USE_CUSTOM_BOUNDARY
    Ds::DsG4OpBoundaryProcessStatus boundary_status = Ds::Undefined ;
#else
    G4OpBoundaryProcessStatus boundary_status = Undefined ;
#endif
 
    unsigned numPoi = m_crec->getNumPoi(); 
    for(unsigned i=0 ; i < numPoi ; i++)
    {
        m_state._step_action = 0 ; 
        CPoi* poi  = m_crec->getPoi(i);

        const G4StepPoint* point = poi->getPoint();
        unsigned flag = poi->getFlag(); 
        unsigned material = poi->getMaterial() ; 
        boundary_status = poi->getBoundaryStatus() ; 

        bool last = i == numPoi - 1 ; 

        bool done = WriteStepPoint( point, flag, material, boundary_status, NULL, last );

        if(done && !last) 
        {
            LOG(LEVEL) 
                << " done and not last "
                << " i " << i 
                << " numPoi " << numPoi
                ;   
        } 

        if(m_dbg) m_dbg->Collect(point, boundary_status, m_photon );
    } 
}


void CRecorder::pointDump( const char* msg, const G4StepPoint* point ) const 
{
    const G4ThreeVector& pos = point->GetPosition();
    const G4ThreeVector& pol = point->GetPolarization();

    LOG(info) << msg 
              << " pos " << std::setw(30) << pos
              << " pol " << std::setw(30) << pol
              ;

}


/**
CRecorder::postTrackWriteSteps
-------------------------------

CRecorder::postTrackWriteSteps is invoked from CRecorder::postTrack in --recstp mode (not --recpoi), 
once for the primary photon track and then for 0 or more reemtracks
via the record_id (which survives reemission) the info is written 
onto the correct place in the photon record buffer

The steps recorded into m_crec(CRec) are used to determine 
flags and the m_state(CRecState) is updated enabling 
appropriate step points are to be saved with WriteStepPoint.


Q: Where does the first flag "SI" "CK" or "TO" come from ?



**/

void CRecorder::postTrackWriteSteps()
{
    assert(!m_live) ;
    LOG(LEVEL) << "[" ; 

#ifdef USE_CUSTOM_BOUNDARY
    LOG(LEVEL) << " USE_CUSTOM_BOUNDARY " ; 
    Ds::DsG4OpBoundaryProcessStatus prior_boundary_status = Ds::Undefined ;
    Ds::DsG4OpBoundaryProcessStatus boundary_status = Ds::Undefined ;
    Ds::DsG4OpBoundaryProcessStatus next_boundary_status = Ds::Undefined ;
#else
    LOG(LEVEL) << " NOT USE_CUSTOM_BOUNDARY " ; 
    G4OpBoundaryProcessStatus prior_boundary_status = Undefined ;
    G4OpBoundaryProcessStatus boundary_status = Undefined ;
    G4OpBoundaryProcessStatus next_boundary_status = Undefined ;
#endif
    bool     done = false  ;  
    bool      SD = false ; 


    unsigned num = m_crec->getNumStp(); 

    bool limited = m_crec->is_step_limited() ; 

    if(m_ctx._dbgrec)
    {
        LOG(LEVEL) 
                  << " [--dbgrec] "
                  << " num " << num
                  << " m_slot " << m_state._slot
                  << " is_step_limited " << ( limited ? "Y" : "N" )
                  << " m_crec.descStages " << m_crec->descStages()
                   ;
    }


    unsigned i = 0 ;  
    for(i=0 ; i < num ; i++)
    {
        m_state._step_action = 0 ; 

        CStp* stp  = m_crec->getStp(i);
        CStp* next_stp = m_crec->getStp(i+1) ;   // NULL for i = num - 1   

        CStage::CStage_t stage = stp->getStage();
        const G4Step* step = stp->getStep();
        const G4StepPoint* pre  = step->GetPreStepPoint() ; 
        const G4StepPoint* post = step->GetPostStepPoint() ; 

        G4ThreeVector delta = step->GetDeltaPosition(); 
        double        step_mm = delta.mag()/mm  ; 
        bool microStep = step_mm <= m_microStep_mm ; 

        //pointDump("CRecorder::postTrackWriteSteps.pre",  pre ); 
        //pointDump("CRecorder::postTrackWriteSteps.post", post ); 


#ifdef USE_CUSTOM_BOUNDARY
        prior_boundary_status = i == 0 ? Ds::Undefined : boundary_status ; 
#else
        prior_boundary_status = i == 0 ? Undefined : boundary_status ; 
#endif

        boundary_status = stp->getBoundaryStatus() ; 

#ifdef USE_CUSTOM_BOUNDARY
        next_boundary_status = next_stp ? next_stp->getBoundaryStatus() : Ds::Undefined ; 
#else
        next_boundary_status = next_stp ? next_stp->getBoundaryStatus() : Undefined ; 
#endif
      
        unsigned premat = m_material_bridge->getPreMaterial(step) ; 

        unsigned postmat = m_material_bridge->getPostMaterial(step) ; 

        bool suppress_microStep = false ; 
        if(m_suppress_same_material_microStep ) suppress_microStep = premat == postmat && microStep ;
        if(m_suppress_all_microStep )           suppress_microStep = microStep ;       
        // suppress_all_microStep trumps suppress_same_material_microStep

        CStage::CStage_t postStage = stage == CStage::REJOIN ? CStage::RECOLL : stage  ; // avoid duping the RE 

        unsigned postFlag = OpStatus::OpPointFlag(post, boundary_status, postStage);

        //if(postFlag == 0 || suppress_microStep )
        if(postFlag == 0)
        {
            LOG(fatal)
                << " num " << num
                << " i " << i 
                << " postFlag zero "
                << " prior_boundary_status " << prior_boundary_status
                << " boundary_status " << boundary_status
                << " next_boundary_status " << next_boundary_status
                << " postStage " << postStage
                << " premat " << premat
                << " postmat " << postmat
                << " suppress_microStep " << ( suppress_microStep ? "YES" : "n" )
                << " step_mm " << step_mm
                << " m_microStep_mm " << m_microStep_mm 
                << Format(delta, "delta", 5 )
                ;
        }


        bool lastPost = (postFlag & (BULK_ABSORB | SURFACE_ABSORB | SURFACE_DETECT | MISS )) != 0 ;

        bool surfaceAbsorb = (postFlag & (SURFACE_ABSORB | SURFACE_DETECT)) != 0 ;
        SD = (postFlag & SURFACE_DETECT) != 0 ;


#ifdef USE_CUSTOM_BOUNDARY
        bool postSkip = ( boundary_status == Ds::StepTooSmall || suppress_microStep ) && !lastPost  ;  
        bool matSwap = next_boundary_status == Ds::StepTooSmall ; 
#else
        bool postSkip = ( boundary_status == StepTooSmall || suppress_microStep ) && !lastPost  ;  
        bool matSwap = next_boundary_status == StepTooSmall ; 
#endif

        // see notes/issues/geant4_opticks_integration/tconcentric_post_recording_has_seqmat_zeros.rst

        if(lastPost)      m_state._step_action |= CAction::LAST_POST ;    // postFlag is AB|SA|SD|MI  (terminal)
        if(surfaceAbsorb) m_state._step_action |= CAction::SURF_ABS ;     // postFlag is SA|SD 
        if(postSkip)      m_state._step_action |= CAction::POST_SKIP ;    // StepTooSmall or microStep being suppressed
        if(matSwap)       m_state._step_action |= CAction::MAT_SWAP ;     // for material bookkeeping after StepTooSmall

        switch(stage)
        {
            case CStage::START:  m_state._step_action |= CAction::STEP_START    ; break ; 
            case CStage::REJOIN: m_state._step_action |= CAction::STEP_REJOIN   ; break ; 
            case CStage::RECOLL: m_state._step_action |= CAction::STEP_RECOLL   ; break ;
            case CStage::COLLECT:                               ; break ; 
            case CStage::UNKNOWN:assert(0)                      ; break ; 
        } 

        bool first = m_state._slot == 0 && stage == CStage::START ;

        unsigned u_premat  = matSwap ? postmat : premat ;
        unsigned u_postmat = ( matSwap || postmat == 0 )  ? premat  : postmat ;

        if(first)            u_premat = premat ;    // dont allow any matswap for 1st material : see notes/issues/tboolean-box-okg4-seqmat-mismatch.rst
        if(surfaceAbsorb)    u_postmat = postmat ; 


        if(stage == CStage::REJOIN) 
        {
             m_state.decrementSlot();   // this allows REJOIN changing of a slot flag from BULK_ABSORB to BULK_REEMIT 
        }

       // as clearStp for each track, REJOIN will always be i=0

        //unsigned preFlag = first ? m_ctx._gen : OpStatus::OpPointFlag(pre,  prior_boundary_status, stage) ;

         
        unsigned preFlag = first ? m_ctx._gs.getGenflag() : OpStatus::OpPointFlag(pre,  prior_boundary_status, stage) ;

        //  preFlag 
        //      former use of m_ctx._genflag was smoking gun cause of lack of CK bug 
        //      m_ctx._genflag was from the last start of genstep which will 
        //      often not be correct at the end of this track 
        //      
        //

        if(preFlag == 0 )
        {
            LOG(fatal) 
                << " preFlag zero " 
                << " first " << first 
                << "  m_ctx._gs.desc  " <<  m_ctx._gs.desc()
                ;          
        }


        if(i == 0) // for 1st step PRE_SAVE and POST_SAVE  
        {

            m_state._step_action |= CAction::PRE_SAVE ; 

            done = WriteStepPoint( pre , preFlag,  u_premat,  prior_boundary_status, PRE, false);   

            if(done) m_state._step_action |= CAction::PRE_DONE ; 

            if(!done)
            {
                 done = WriteStepPoint( post, postFlag, u_postmat, boundary_status,       POST, false );  

                 m_state._step_action |= CAction::POST_SAVE ; 

                 if(done) m_state._step_action |= CAction::POST_DONE ; 
            }
        }
        else     // after 1st step just POST_SAVE 
        {
            if(!postSkip && !done)
            {
                m_state._step_action |= CAction::POST_SAVE ; 

                done = WriteStepPoint( post, postFlag, u_postmat, boundary_status, POST, false );

                if(done) m_state._step_action |= CAction::POST_DONE ; 
            }
        }

        // huh: changing the inputs (for dumping presumably) ??? nevertheless confusing 
        // ... but are using next step lookahead, 
        stp->setMat(  u_premat, u_postmat );
        stp->setFlag( preFlag,  postFlag );
        stp->setAction( m_state._step_action );


        if(m_ctx._dbgrec)
            LOG(info) << "[--dbgrec] postTrackWriteSteps " 
                      << "[" << std::setw(2) << i << "]"
                      << " action " << getStepActionString()
                      ;


        if(done) break ; 


    }   // stp loop




    if(SD)
    {
        LOG(LEVEL) 
            << " SD " << SD 
            << " done " << done  
            << " m_ctx._pho.desc " << m_ctx._pho.desc() 
            << " m_photon.desc " << m_photon.desc() 
            ;
    }



    if(!done)
    {
        m_not_done_count++ ; 
        LOG(LEVEL) << "postTrackWriteSteps  not-done " 
                   << m_not_done_count
                   << " photon " << m_photon.desc()
                   << " action " << getStepActionString()
                   << " i " << i 
                   << " num " << num 
                   ; 
    } 
    LOG(LEVEL) << "]" ; 
}




/**
CRecorder::WriteStepPoint
---------------------------

* In --recpoi mode is invoked from CRecorder::postTrackWritePoints very simply.
* In --recstp mode is invoked several times from CRecorder::postTrackWriteSteps.

NB the last argumnent is only relevant to --recpoi mode

Passes thru to m_writer CWriter::writeStepPoint 

**/


#ifdef USE_CUSTOM_BOUNDARY
bool CRecorder::WriteStepPoint(const G4StepPoint* point, unsigned flag, unsigned int material, Ds::DsG4OpBoundaryProcessStatus boundary_status, const char*, bool last )
{
    if(flag == 0)
    {
        if(!(boundary_status == Ds::SameMaterial || boundary_status == Ds::Undefined))
            LOG(warning) << " boundary_status not handled : " << OpStatus::OpBoundaryAbbrevString(boundary_status) ; 
    }
    // the below adds flag and material to the shared m_photon struct
    return m_writer->writeStepPoint( point, flag, material, last );
}
#else
bool CRecorder::WriteStepPoint(const G4StepPoint* point, unsigned flag, unsigned int material, G4OpBoundaryProcessStatus boundary_status, const char*, bool last )
{
    if(flag == 0)
    {
        if(!(boundary_status == SameMaterial || boundary_status == Undefined))
            LOG(warning) << " boundary_status not handled : " << OpStatus::OpBoundaryAbbrevString(boundary_status) ; 
    }
    // the below adds flag and material to the shared m_photon struct
    return m_writer->writeStepPoint( point, flag, material, last );
}
#endif


/*
        if(m_ctx._debug || m_ctx._other)
        {
            LOG(info)
                  << ( m_state._topslot_rewrite > 1 ? CAction::HARD_TRUNCATE_ : CAction::TOPSLOT_REWRITE_ )
                  << " topslot_rewrite " << m_state._topslot_rewrite
                  << " prior_flag -> flag " 
                  <<   OpticksFlags::Abbrev(m_photon._flag_prior)
                  << " -> " 
                  <<   OpticksFlags::Abbrev(flag)
                  << " prior_mat -> mat " 
                  <<   ( m_photon._mat_prior == 0 ? "-" : m_material_bridge->getMaterialName(m_photon._mat_prior-1, true)  ) 
                  << " -> "
                  <<   ( m_photon._mat == 0       ? "-" : m_material_bridge->getMaterialName(m_photon._mat-1, true)  ) 
                  ;
        } 

*/





void CRecorder::Summary(const char* msg)
{
    LOG(info) <<  msg << " " << desc() ;
}

void CRecorder::dump(const char* msg)
{
    LOG(info) << msg ; 
    m_crec->dump("CRec::dump");

    if(m_dbg)
    m_dbg->dump("CRecorder::dump");
}

std::string CRecorder::desc() const 
{
    std::stringstream ss ; 
    ss << std::setw(10) << CStage::Label(m_ctx._stage)
       << " evt " << std::setw(7) << m_ctx._event_id
       << " pho " << std::setw(7) << m_ctx._photon_id 
       << " pri " << std::setw(7) << m_ctx._primary_id
       << " ste " << std::setw(4) << m_ctx._step_id 
       << " rid " << std::setw(4) << m_ctx._record_id 
       << " slt " << std::setw(4) << m_state._slot
       << " pre " << std::setw(7) << CStep::PreGlobalTime(m_ctx._step)
       << " pst " << std::setw(7) << CStep::PostGlobalTime(m_ctx._step)
       ;

   return ss.str();
}


