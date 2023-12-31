/**
G4CXSimtraceTest.cc :  based on cx:tests/CSGOptiXSimtraceTest.cc
==========================================================================

Canonically used from g4cx/gxt.sh 

**/

#include <cuda_runtime.h>

#include "OPTICKS_LOG.hh"
#include "SEventConfig.hh"
#include "G4CXOpticks.hh"

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    //LOG(info) << "[ cu first " ; 
    cudaDeviceSynchronize(); 
    //LOG(info) << "] cu first " ; 

    SEventConfig::SetRGModeSimtrace();   

    G4CXOpticks* cx = G4CXOpticks::SetGeometry() ;  
    cx->simtrace(0); 

 
    return 0 ; 
}
