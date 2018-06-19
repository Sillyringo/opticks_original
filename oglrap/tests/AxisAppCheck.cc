#include "Opticks.hh"
#include "AxisApp.hh"

#include "OPTICKS_LOG.hh"

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    LOG(info) << argv[0] ; 

    Opticks ok(argc, argv);

    AxisApp aa(&ok); 

    aa.renderLoop();

    return 0 ; 
}


//
// runs, but currently producing a black window ... maybe missing the color buffer
// in current OpticksViz without geometry 
//

