/**
GeoChainVolumeTest.cc : testing the full chain of geometry conversions
=========================================================================

The volume to create is controlled by the name string obtained from envvar *GEOCHAINTEST* 

**/

#include <cassert>
#include "OPTICKS_LOG.hh"
#include "SSys.hh"
#include "SStr.hh"
#include "Opticks.hh"
#include "GeoChain.hh"

#include "G4VPhysicalVolume.hh"

#ifdef WITH_PMTSIM
#include "PMTSim.hh"
#endif

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 
    const char* name_default = "body_phys"  ; 
    const char* name = SSys::getenvvar("GEOM", name_default ); 

    std::stringstream ss ; 
    ss << "creator:GeoChainVolumeTest" << std::endl ; 
    ss << "name:" << name << std::endl ; 
    std::string meta = ss.str(); 

    const G4VPhysicalVolume* pv = nullptr ; 
#ifdef WITH_PMTSIM
    pv = PMTSim::GetPV(name);  
#endif
    assert( pv ); 

    const char* argforced = "--allownokey --gparts_transform_offset" ; 
    Opticks ok(argc, argv, argforced); 
    ok.configure(); 

    GeoChain chain(&ok); 
    chain.convertPV(pv, meta);  
    chain.save(name); 

    return 0 ; 
}
