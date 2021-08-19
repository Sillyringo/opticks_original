#pragma once

struct quad6 ;
template <typename T> struct SBuf ; 
#include "QUDARAP_API_EXPORT.hh"

/**
QSeed
======

The photon seed buffer is a device buffer containing integer indices referencing 
into the genstep buffer. The seeds provide the association between the photon 
and the genstep required to generate it.

**/

struct QUDARAP_API QSeed
{
    static SBuf<int> CreatePhotonSeeds(SBuf<quad6> gs); 
};


