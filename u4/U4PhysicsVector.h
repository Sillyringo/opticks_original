#pragma once

#include "G4PhysicsOrderedFreeVector.hh"
#include "G4PhysicsVector.hh"
#include "G4PhysicsTable.hh"
#include "G4SystemOfUnits.hh"

#include "NP.hh"

struct U4PhysicsVector
{
    static G4PhysicsOrderedFreeVector* CreateConst(double value); 

    static NP* ConvertToArray(const G4PhysicsVector* prop) ;   
    static NP* CreateCombinedArray( const G4PhysicsTable* table ); 
}; 


G4PhysicsOrderedFreeVector* U4PhysicsVector::CreateConst(double value)
{
    G4double Energies[2] = { 1.55*eV , 15.5*eV } ; 
    G4double Values[2]   = { value, value } ; 
    return new G4PhysicsOrderedFreeVector(Energies, Values, 2 ) ; 
}

/**

HMM same as U4MaterialPropertyVector::ConvertToArray( const G4MaterialPropertyVector* prop )

**/

inline NP* U4PhysicsVector::ConvertToArray(const G4PhysicsVector* prop) // static 
{
    size_t num_val = prop ? prop->GetVectorLength() : 0 ; 
    NP* a = NP::Make<double>( num_val, 2 );
    double* a_v = a->values<double>(); 
    for(size_t i=0 ; i < num_val ; i++)
    {   
        G4double energy = prop->Energy(i); 
        G4double value = (*prop)[i] ;
        a_v[2*i+0] = energy ; 
        a_v[2*i+1] = value ; 
    }       
    return a ;   
}

inline NP* U4PhysicsVector::CreateCombinedArray( const G4PhysicsTable* table )
{
    if(table == nullptr) return nullptr ; 
    size_t entries = table->entries() ; 
    if(entries == 0) return nullptr ; 

    const G4PhysicsTable& tab = *table ; 

    std::vector<const NP*> aa ; 
    for(size_t i=0 ; i < entries ; i++)
    {
        G4PhysicsVector* vec = tab(i) ; 
        const NP* a = ConvertToArray(vec) ; 
        aa.push_back(a);  
    }
    return NP::Combine(aa); 
}





