/**
G4CXOpticks_setGeometry_Test.cc
=================================

Action depends on envvars such as OpticksGDMLPath, see G4CXOpticks::setGeometry

The hookup of the sframe obtained from CSGFoundry geometry with SEvt 
is done by G4CXOpticks::setupFrame which gets invoked by G4CXOpticks::setGeometry

**/

#include "OPTICKS_LOG.hh"
#include "G4CXOpticks.hh"
#include "SEvt.hh"
#include "sstr.h"
#include "ssys.h"

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    G4CXOpticks::SetGeometry();  

    LOG(info) << SEvt::Brief() ; 

    NP* ip = SEvt::GetInputPhoton(0) ; 

    LOG_IF(error, ip == nullptr) << "SEvt::GetInputPhoton GIVES nullptr : SET OPTICKS_INPUT_PHOTON TO CONFIGURE " ; 

    const char* id = SEvt::GetFrameId(0) ; 
    const NP*   fr = SEvt::GetFrameArray(0) ; 

    std::string ip_name = sstr::Format_("ip_%s.npy", ( id ? id : "noid" ) ); 
    std::string fr_name = sstr::Format_("fr_%s.npy", ( id ? id : "noid" ) ); 

    LOG(info) 
        << " id " << id 
        << " ip_name " << ip_name
        << " fr_name " << fr_name
        ; 

    if(fr) fr->save("$FOLD", fr_name.c_str()) ; 
    if(ip) ip->save("$FOLD", ip_name.c_str()) ; 

    return 0 ; 
}
