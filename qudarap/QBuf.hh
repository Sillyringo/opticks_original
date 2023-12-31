#pragma once
/**
QBuf.hh : header only CUDA device buffer
------------------------------------------

Hmm: with creater used of QU am unsure regards QBuf ?


**/

#include <string>
#include <iostream>
#include <sstream>
#include <cassert>
#include <vector>

#include "QUDA_CHECK.h"
#include "QUDARAP_API_EXPORT.hh"
#include "NP.hh"


template <typename T>
struct QUDARAP_API QBuf 
{
    void* a ; 
    T* h ; 
    T* d ; 
    unsigned num_items ; 
    unsigned max_items ; 

    QBuf()
        :
        a(nullptr),   // will often be an NP array 
        h(nullptr),
        d(nullptr),
        num_items(0),
        max_items(0)
    {
    }


    void device_alloc(unsigned num_items_)
    {   
        num_items = num_items_ ; 
        QUDA_CHECK(cudaMalloc(reinterpret_cast<void**>( &d ), num_items*sizeof(T) ));  
    }   
    void device_set(int value=0)  // Value to set for each byte of specified memory
    {   
        QUDA_CHECK(cudaMemset(reinterpret_cast<void*>( d ), value, num_items*sizeof(T)  ));  
    }   
    void device_free()
    {   
        QUDA_CHECK(cudaFree(d)) ; 
        d = nullptr ; 
        num_items = 0 ; 
    }   
    std::string desc() const 
    {   
        std::stringstream ss ; 
        ss <<  "QBuf d " << ( d ? d : 0 ) << " num_items " << num_items ; 
        return ss.str();   
    }   
    void upload( const T* data, unsigned num_items_ )
    {   
        if( num_items > 0 ) assert( num_items_ == num_items );  
        QUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>( d ), data, sizeof(T)*num_items_, cudaMemcpyHostToDevice )) ; 
    }   

    void download( std::vector<T>& vec )
    {
        vec.resize(num_items);  
        QUDA_CHECK(cudaMemcpy(reinterpret_cast<void*>( vec.data() ), d , sizeof(T)*num_items, cudaMemcpyDeviceToHost ));
    }

    void download_dump(const char* msg, unsigned edgeitems);
 
    static QBuf<T>* Upload( const T* data, unsigned num_items )
    {   
        QBuf<T>* buf = new QBuf<T>() ; 
        buf->device_alloc(num_items);   // sets (ptr, num_items)
        buf->upload( data, num_items );  
        return buf ; 
    }   

    // caution : this is allocating every time
    // for event by event handling better to allocate one and resize ?
    static QBuf<T>* Upload( const NP* a  )
    {   
        return Upload( a->cvalues<T>(), a->num_values() );  
    }   

    static QBuf<T>* Upload( const std::vector<T>& vec  )
    {   
        return Upload( vec.data(), vec.size() );  
    }   

    /**
    method tickles CUDA/cxx17/devtoolset-8 bug causing compilation to fail with 
    error: cannot call member function without object

    See notes/issues/cxx17_issues.rst

    https://forums.developer.nvidia.com/t/cuda-10-1-nvidia-youre-now-fixing-gcc-bugs-that-gcc-doesnt-even-have/71063

    **/

    static QBuf<T>* Alloc( unsigned num_items  )
    {   
        QBuf<T>* buf = new QBuf<T> ; 
        (*buf).device_alloc(num_items); 
        (*buf).device_set(0); 
        return buf ; 
    }   
};



template<typename T> 
inline void QBuf<T>::download_dump(const char* msg, unsigned )
{
    std::cout << "QBuf::download_dump " << msg << " PLACEHOLDER " << std::endl ; 
}

template<> 
inline void QBuf<int>::download_dump(const char* msg, unsigned edgeitems)
{
    std::vector<int> chk  ;   
    download(chk); 
    std::cout << "QBuf::download_dump " << msg << std::endl ; 
    for(unsigned i=0 ; i < chk.size() ; i++ ) 
    {
        if( i < edgeitems || i > chk.size() - edgeitems ) std::cout << chk[i] << " "  ;   
        else if ( i == edgeitems ) std::cout << "... " ; 
    }
    std::cout << std::endl ; 
}



