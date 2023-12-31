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
#include <sstream>

#include "SSys.hh"     // sysrap- 

#include "NPY.hpp"      // npy-
#include "FabStepNPY.hpp"

#include "Opticks.hh"   // okc-
#include "OpticksPhoton.h"   
#include "OpticksEvent.hh"

#include "OpticksHub.hh"  // okg-
#include "OpticksRun.hh"  
#include "OpticksGen.hh"  

#include "OContext.hh"   // optixrap-
#include "OPropagator.hh"   
#include "OScene.hh" 
#include "OEvent.hh" 

#include "OpSeeder.hh"   // okop-

#include "OPTICKS_LOG.hh"

/**
OpSeederTest
============

**/


struct OpSeederTest 
{
    OpSeederTest( int argc, char** argv)
    {
        const char* forced = "--compute --trivial --multievent 2 --machinery" ;
        Opticks ok(argc, argv, forced );

        OpticksHub hub(&ok);
        OpticksGen* gen = hub.getGen();

        LOG(info) << " EntryCode: " << ok.getEntryCode() 
                  << " EntryName: " << ok.getEntryName()
                  << " ArgLine: " << ok.getArgLine()
                  ;
        
        assert(ok.isCompute());

        OScene scene(&hub);
        OContext* octx = scene.getOContext();
        OEvent oevt(&ok, octx );
        OpSeeder  seeder(&ok, &oevt );
        OPropagator propagator(&ok, &oevt, octx->addEntry(ok.getEntryCode(),"OpSeederTest::OpSeederTest" ) );

        FabStepNPY* fab = gen->makeFabstep();
        NPY<float>* gs = fab->getNPY();

        bool compute = true ; 
        gs->setBufferSpec(OpticksEvent::GenstepSpec(compute));  
     

        int multi = ok.getMultiEvent();
        char entryCode = ok.getEntryCode();

        char ctrl = '+' ; 

        for(int i=0 ; i < multi ; i++)
        {
            ok.createEvent(gs, ctrl);

            OpticksEvent* evt = ok.getEvent();
            assert(evt->isMachineryType() && "--machinery type is forced as this writes non-standardOpticksEvents which would otherwise cause test failures for event reading tests" ); 


            oevt.upload();                        // uploads gensteps, creates buffers at 1st upload, resizes on subsequent uploads

            //propagator.prelaunch();               // sticking a pre-launch here, succeeds to give OptiX the hint that smth changed
                                                  // and Thrust reading of gensteps and writing of seeds manages to be 
                                                  // seen by OptiX, otherwise the seeds are still written correctly by Thrust 
                                                  // but OptiX doesnt notice
                                                  // .... so this manages to get Thrust and OptiX to share the buffers 
                                                  //
                                                  //   HUH: now I remove this and still working ???


            seeder.seedPhotonsFromGensteps() ;    // Thrust: seed photon buffer using the genstep numPhotons for each step

            oevt.markDirty();                     // inform OptiX that must sync buffers that are using ctrl: BUFFER_COPY_ON_DIRTY

            propagator.launch();                  // write the photon, record and sequence buffers

            oevt.download();

            if(entryCode == 'T' || entryCode == 'D') 
            {
                int sdc = evt->seedDebugCheck("OpSeederTest");
                if(sdc != 0) LOG(fatal) << "seedDebugCheck FAIL " << sdc ;  
                assert(sdc == 0); 
            }

            evt->save();

           // TODO: arrange this to use standard anakey running of analysis scripts
            SSys::npdump(evt->getPath("photon"), "np.int32");

        }
        ok.postpropagate();
    }
};


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);    

    OpSeederTest ost(argc, argv); 

    return 0 ;     
}

/*

    OpSeederTest --compute    
       # this gives random bits for the photons, 
       # photon buffers on the device are allocated but as the fabricated gensteps are incomplete
       # ray traces will always miss geometry resulring in misses so psave will 
       # record random content of GPU stack ?
       #
       # Fabricated gensteps just have photon count and a fake materialline
       # so they will not generate photons that can hit geometry
       #
       # MAYBE: introduce a FABRICATED source type which minimally sets up some photons
       # but without the complication of TORCH ?
       #

    OpSeederTest  --compute --nopropagate
    OpSeederTest  --compute --trivial --nopropagate
       # actual seeds at the head of the photons are visible with the rest of 
       # the buffer being zero 
       # lack of real gensteps doesnt matter as propagation is skipped
       #
       # The rest of the buffer being zero is due to the filling of missers
       # never being done


    OpSeederTest  --compute --trivial
       # trivial debug lines are visible

    OpSeederTest  --compute --trivial --multievent 2
    OpSeederTest  --compute --trivial --multievent 10
       #
       # (previously the 2nd event has genstep id stuck at zero, before adopting 
       #  BUFFER_COPY_ON_DIRTY and manual markDirtyPhotonBuffer)
       #
       #  now the 2nd and subsequent events have proper seeds 

    OpSeederTest  --compute --nopropagate --multievent 2
       #
       # huh: unexpectedly the seeds ARE BEING SET FOR THE 2nd event (when the propagate launch is skipped altogether)
       # IT IS STILL NOT UNDERSTOOD WHY THE SYNC SOMETIMES DID AND SOMETIMES DIDNT HAPPEN

    OpSeederTest  --compute --nothing --multievent 2
       # try launching but running a do nothing program 
       # again the seeds ARE being set, but there is no access to the buffer
       # so OptiX may be being clever and skipping a buffer upload 

    OpSeederTest  --compute --dumpseed --multievent 2
       # just dumping the indices in quad3 (and not touching the quad0 location of the seed)
       # demonstrates again that genstep_id are being seeded correctly for the 
       # 2nd event (just like the 1st)
       #
       # problem is that the 2nd seeding doesnt overwrite the zeroing       
 
    OpSeederTest  --compute --trivial --multievent 2
       # after changing to use BUFFER_COPY_ON_DIRTY and invoking  oevt.markDirtyPhotonBuffer(); 
       # above multievent now appears to see the changed seeds
       #
       # but this approach not working in interop, so have got WITH_SEED_BUFFER
       # approach working in compute,  but not yet in INTEROP

    OpSeederTest --dumpseed



*/



