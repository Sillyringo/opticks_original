#pragma once

#include <string>
#include <iomanip>
#include "Randomize.hh"
#include "NP.hh"

struct U4UniformRand
{
    static std::string Desc(int n=10); 
    static NP* Get(int n=1000); 
}; 

inline std::string U4UniformRand::Desc(int n )
{
    std::stringstream ss ; 
    ss << "U4UniformRand::Desc" << std::endl ; 
    for(int i=0 ; i < n ; i++)
    {
        G4double u = G4UniformRand(); 
        ss << std::setw(6) << i 
           << " " << std::setw(10) << std::fixed << std::setprecision(5) << u 
           << std::endl 
           ;
    }
    std::string s = ss.str(); 
    return s; 
}

inline NP* U4UniformRand::Get(int n)
{
    NP* u = NP::Make<double>(n); 
    double* uu = u->values<double>(); 
    for(int i=0 ; i < n ; i++) uu[i] = G4UniformRand();
    return u ; 
}


