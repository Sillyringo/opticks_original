#include "SSys.hh"
#include "OPTICKS_LOG.hh"
#include "X4VolumeMaker.hh"

int main(int argc, char** argv)
{  
    OPTICKS_LOG(argc, argv); 

    //const char* geom_default = "JustOrbGrid" ; 
    //const char* geom_default = "JustOrbCube" ;
    const char* geom_default = "ListJustOrb,BoxMinusOrb" ;
 
    const char* geom = SSys::getenvvar("X4VolumeMakerTest_GEOM", geom_default ); 
    LOG(info) << " geom " << geom ; 

    const G4VPhysicalVolume* pv = X4VolumeMaker::MakePhysical( geom ); 
    assert( pv ); 

    return 0 ; 
}
