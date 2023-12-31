#pragma once

#if defined(__CUDACC__) || defined(__CUDABE__)
   #define SCURAND_METHOD __device__
   #include "curand_kernel.h"
#else
   #define SCURAND_METHOD 

#if defined(MOCK_CURAND) || defined(MOCK_CUDA)
   #include "s_mock_curand.h"
#else
   #include "srng.h"
#endif

#endif 

template <typename T>
struct scurand
{
#if defined(__CUDACC__) || defined(__CUDABE__) || defined(MOCK_CURAND) || defined(MOCK_CUDA)
   static SCURAND_METHOD T uniform( curandStateXORWOW* rng );  
#endif
};


#if defined(__CUDACC__) || defined(__CUDABE__) || defined(MOCK_CURAND) || defined(MOCK_CUDA)

template<> inline float scurand<float>::uniform( curandStateXORWOW* rng ) 
{ 
#ifdef FLIP_RANDOM
    return 1.f - curand_uniform(rng) ;
#else
    return curand_uniform(rng) ;
#endif
}

template<> inline double scurand<double>::uniform( curandStateXORWOW* rng ) 
{ 
#ifdef FLIP_RANDOM
    return 1. - curand_uniform_double(rng) ;
#else
    return curand_uniform_double(rng) ;
#endif
}

#endif


