/**
U4SimulateTest.cc ( formerly U4PMTFastSimTest.cc)
======================================================

Most of the Geant4 setup happens on instanciating U4App from U4App.h

**/

#include "ssys.h"
#include "U4App.h"    
#include "STime.hh"
#include "SEvt.hh"

#ifdef WITH_PMTSIM

#include "J_PMTSIM_LOG.hh"
#include "PMTSim.hh"

#endif

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

#ifdef WITH_PMTSIM
    J_PMTSIM_LOG_(0); 
#endif

    int VERSION = ssys::getenvint("VERSION", 0 );  
    LOG(info) << "[ " << argv[0] << " " << STime::Now() << " VERSION " << VERSION ; 

    SEvt* evt = SEvt::HighLevelCreate(); 

    U4App* app = U4App::Create() ;  
    app->BeamOn(); 

    evt->save(); 
    const char* savedir = evt->getSaveDir(); 
    U4App::SaveMeta(savedir); 

#if defined(WITH_PMTSIM) && defined(POM_DEBUG)
    PMTSim::ModelTrigger_Debug_Save(savedir) ; 
#else
    LOG(info) << "not-POM_DEBUG  "  ; 
#endif

    delete app ; 

    LOG(info) << "] " << argv[0] << " " << STime::Now() << " VERSION " << VERSION << " savedir " << savedir ; 
    return 0 ; 
}

