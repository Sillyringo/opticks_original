#pragma once

/**
sview.h
=========

Templated reinterpretation of bits allowing to view 
unsigned int as float and double and vice versa.

**/

#if defined(__CUDACC__) || defined(__CUDABE__)
#    define SVIEW_METHOD __host__ __device__ __forceinline__
#else
#    define SVIEW_METHOD inline
#endif

struct sview
{
    union UIF32 
    {
        unsigned u ;  
        int i  ;  
        float f ; 
    }; 

    struct uint2 { unsigned x, y ; }; 
    struct int2  { unsigned x, y ; }; 

    union UIF64
    {
        uint2  uu ;  
        int2   ii ;  
        double f ; 
    }; 

    template<typename T> static T int_as( int i ); 
    template<typename T> static int int_from( T v ); 

    template<typename T> static T uint_as( unsigned u ); 
    template<typename T> static unsigned uint_from( T v ); 
}; 

template<> SVIEW_METHOD float sview::int_as<float>( int i )
{
     UIF32 u32 ; 
     u32.i = i ; 
     return u32.f ; 
}
template<> SVIEW_METHOD double sview::int_as<double>( int i )
{
     UIF64 u64 ; 
     u64.ii.x = i ; 
     return u64.f ; 
}


template<> SVIEW_METHOD int sview::int_from<float>( float f )
{
     UIF32 u32 ; 
     u32.f = f ; 
     return u32.i ; 
}
template<> SVIEW_METHOD int sview::int_from<double>( double f )
{
     UIF64 u64 ; 
     u64.f = f ; 
     return u64.ii.x  ; 
}

template<> SVIEW_METHOD float sview::uint_as<float>( unsigned u )
{
     UIF32 u32 ; 
     u32.u = u ; 
     return u32.f ; 
}
template<> SVIEW_METHOD double sview::uint_as<double>( unsigned v )
{
     UIF64 u64 ; 
     u64.uu.x = v ; 
     return u64.f ; 
}

template<> SVIEW_METHOD unsigned sview::uint_from<float>( float f )
{
     UIF32 u32 ; 
     u32.f = f ; 
     return u32.u ; 
}
template<> SVIEW_METHOD unsigned sview::uint_from<double>( double f )
{
     UIF64 u64 ; 
     u64.f = f ; 
     return u64.uu.x ; 
}

