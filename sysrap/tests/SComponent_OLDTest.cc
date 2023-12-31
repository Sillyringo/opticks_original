// name=SComponent_OLDTest ; gcc $name.cc -g -std=c++11 -lstdc++ -I.. -o /tmp/$name && lldb__ /tmp/$name

#include <vector>
#include <iostream>
#include <iomanip>
#include "OpticksGenstep.h"

#include "SComponent_OLD.hh"

int main(int argc, char** argv)
{
    std::vector<const char*> names = {"photon.npy", "genstep.npy", "hit.npy" } ; 

    for(unsigned i=0 ; i < 20 + names.size() ; i++) 
    {
        const char* name = i < 20 ? SComponent_OLD::Name(i) : names[i-20] ;  
        unsigned comp = SComponent_OLD::Component(name); 
        const char* compname = SComponent_OLD::Name(comp); 
        std::cout 
            << " i " << std::setw(3) << i 
            << " name " << std::setw(20) << ( name ? name : "-" )
            << " comp " << std::setw(3)   << comp 
            << " compname " << std::setw(20) << ( compname ? compname : "-" )
            << std::endl 
            ; 
    }
    return 0 ; 
}

