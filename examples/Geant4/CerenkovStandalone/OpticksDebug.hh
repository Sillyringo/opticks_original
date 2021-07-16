#pragma once

#include <vector>
#include <string>


struct NP ; 
#include "G4MaterialPropertyVector.hh"
class G4Material ; 


struct UU
{
   unsigned x ; 
   unsigned y ; 
};

union DUU
{
   double d ; 
   UU     uu ; 
};


struct OpticksDebug
{
    unsigned itemsize ; 
    const char* name ; 

    OpticksDebug(unsigned itemsize, const char* name); 

    static std::string prepare_path(const char* dir_, const char* reldir_, const char* name );
    static NP* LoadArray(const char* kdpath);
    static G4MaterialPropertyVector* MakeProperty(const NP* a);
    static G4Material* MakeMaterial(G4MaterialPropertyVector* rindex) ; 

    std::vector<std::string> names ; 
    std::vector<double>      values ; 

    void append( double x, const char* name );
    void append( unsigned x, unsigned y, const char* name ); 
    void write(const char* dir, const char* reldir, unsigned nj, unsigned nk ) ; 

};

