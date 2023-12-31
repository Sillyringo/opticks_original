/*
 * Copyright (c) 2019 Opticks Team. All Rights Reserved.
 *
 * This file is part of Opticks
 * (see https://bitbucket.org/simoncblyth/opticks).
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */

#include <iostream>
#include <sstream>
#include "NPYBase.hpp"
#include "OBufBase.hh"
#include "OFormat.hh"

OBufBase::OBufBase(const char* name, optix::Buffer& buffer) 
   :
   m_buffer(buffer), 
   m_name(strdup(name)), 
   m_multiplicity(0ull), 
   m_sizeofatom(0ull), 
   m_device(0u),
   m_hexdump(false)
{
    init();
}

OBufBase::~OBufBase()
{
   // no owned resources worth clearing up
}


CBufSlice OBufBase::slice( unsigned long long stride, unsigned long long begin, unsigned long long end )
{
   return CBufSlice( getDevicePtr(), getSize(), getNumBytes(), stride, begin, end == 0u ? getNumAtoms() : end);
}

CBufSpec OBufBase::bufspec()
{
   return CBufSpec( getDevicePtr(), getSize(), getNumBytes()) ;
}

void* OBufBase::getDevicePtr() 
{
    CUdeviceptr cu_ptr = (CUdeviceptr)m_buffer->getDevicePointer(m_device) ;
    return (void*)cu_ptr ; 
}





std::string OBufBase::desc() const 
{
   std::stringstream ss ;  
   ss << "OBufBase" 
      << " name: " << m_name
      << " size: " << getSize()
      << " multiplicity: " << m_multiplicity
      << " sizeofatom: " << m_sizeofatom
      << " NumAtoms: " << getNumAtoms()
      << " NumBytes: " << getNumBytes()
      ; 
 
   return ss.str(); 
}



void OBufBase::Summary(const char* msg) const 
{
    printf("%s name %s size %llu multiplicity %llu sizeofatom %llu NumAtoms %llu NumBytes %llu \n", 
         msg, 
         m_name, 
         getSize(), 
         m_multiplicity, 
         m_sizeofatom, 
         getNumAtoms(), 
         getNumBytes() );
}

void OBufBase::setHexDump(bool hexdump)
{
   m_hexdump = hexdump ; 
}

/**
OBufBase::getSize
----------------------

Excludes multiplicity of the type of the OptiX buffer, ie the size
is the number of float4 

Examples:

1) Cerenkov genstep NPY<float> buffer with dimensions (7836,6,4)
   is canonically represented as an OptiX float4 buffer of size 7836*6 = 47016 

2) Torch genstep NPY<float> buffer with dimensions (1,6,4)
   is canonically represented as an OptiX float4 buffer of size 1*6 = 6 

**/

unsigned long long OBufBase::getSize() const 
{
    return Size(m_buffer) ; 
}

unsigned long long OBufBase::getMultiplicity() const 
{
    return m_multiplicity ; 
}
unsigned long long OBufBase::getNumAtoms() const  
{
    return getSize()*m_multiplicity ; 
}
unsigned long long OBufBase::getSizeOfAtom() const 
{
    return m_sizeofatom ; 
}
unsigned long long OBufBase::getNumBytes() const 
{
    return NumBytes(m_buffer) ; 
}

void OBufBase::init()
{
    examineBufferFormat(m_buffer->getFormat());
}

void OBufBase::examineBufferFormat(RTformat format)
{
   unsigned long long mul(0) ;
   unsigned long long soa(0) ;
   bool unknown(false); 
   //std::cout << "OBufBase::examineBufferFormat " << format << std::endl  ; 

   switch(format)
   {   
      case RT_FORMAT_UNKNOWN: mul=0 ;soa=0 ;  break ; 

      case RT_FORMAT_FLOAT:   mul=1 ; soa=sizeof(float) ; break ;
      case RT_FORMAT_FLOAT2:  mul=2 ; soa=sizeof(float) ; break ;
      case RT_FORMAT_FLOAT3:  mul=3 ; soa=sizeof(float) ; break ;
      case RT_FORMAT_FLOAT4:  mul=4 ; soa=sizeof(float) ; break ;

      case RT_FORMAT_BYTE:    mul=1 ; soa=sizeof(char)  ; break ;
      case RT_FORMAT_BYTE2:   mul=2 ; soa=sizeof(char)  ; break ;
      case RT_FORMAT_BYTE3:   mul=3 ; soa=sizeof(char)  ; break ;
      case RT_FORMAT_BYTE4:   mul=4 ; soa=sizeof(char)  ; break ;

      case RT_FORMAT_UNSIGNED_BYTE:  mul=1 ; soa=sizeof(unsigned char) ; break ;
      case RT_FORMAT_UNSIGNED_BYTE2: mul=2 ; soa=sizeof(unsigned char) ; break ;
      case RT_FORMAT_UNSIGNED_BYTE3: mul=3 ; soa=sizeof(unsigned char) ; break ;
      case RT_FORMAT_UNSIGNED_BYTE4: mul=4 ; soa=sizeof(unsigned char) ; break ;

      case RT_FORMAT_SHORT:  mul=1 ; soa=sizeof(short) ; break ;
      case RT_FORMAT_SHORT2: mul=2 ; soa=sizeof(short) ; break ;
      case RT_FORMAT_SHORT3: mul=3 ; soa=sizeof(short) ; break ;
      case RT_FORMAT_SHORT4: mul=4 ; soa=sizeof(short) ; break ;

      case RT_FORMAT_UNSIGNED_SHORT:  mul=1 ; soa=sizeof(unsigned short) ; break ;
      case RT_FORMAT_UNSIGNED_SHORT2: mul=2 ; soa=sizeof(unsigned short) ; break ;
      case RT_FORMAT_UNSIGNED_SHORT3: mul=3 ; soa=sizeof(unsigned short) ; break ;
      case RT_FORMAT_UNSIGNED_SHORT4: mul=4 ; soa=sizeof(unsigned short) ; break ;

      case RT_FORMAT_INT:  mul=1 ; soa=sizeof(int) ; break ;
      case RT_FORMAT_INT2: mul=2 ; soa=sizeof(int) ; break ;
      case RT_FORMAT_INT3: mul=3 ; soa=sizeof(int) ; break ;
      case RT_FORMAT_INT4: mul=4 ; soa=sizeof(int) ; break ;

      case RT_FORMAT_UNSIGNED_INT:  mul=1 ; soa=sizeof(unsigned int) ; break ;
      case RT_FORMAT_UNSIGNED_INT2: mul=2 ; soa=sizeof(unsigned int) ; break ;
      case RT_FORMAT_UNSIGNED_INT3: mul=3 ; soa=sizeof(unsigned int) ; break ;
      case RT_FORMAT_UNSIGNED_INT4: mul=4 ; soa=sizeof(unsigned int) ; break ;

      case RT_FORMAT_USER:       mul=0 ; soa=0 ; break ;
      case RT_FORMAT_BUFFER_ID:  mul=0 ; soa=0 ; break ;
      case RT_FORMAT_PROGRAM_ID: mul=0 ; soa=0 ; break ; 

#if OPTIX_VERSION >= 4000
      case RT_FORMAT_HALF  : mul=1 ; soa=sizeof(float)/2 ; break ; 
      case RT_FORMAT_HALF2 : mul=2 ; soa=sizeof(float)/2 ; break ; 
      case RT_FORMAT_HALF3 : mul=3 ; soa=sizeof(float)/2 ; break ; 
      case RT_FORMAT_HALF4 : mul=4 ; soa=sizeof(float)/2 ; break ; 
#endif
      default:   unknown = true  ;  
   }   

    unsigned long long element_size_bytes = OFormat::ElementSizeInBytes(format);

    bool expected = element_size_bytes == soa*mul && !unknown  ; 
    if(!expected ) 
          std::cerr 
              << " format " << OFormat::FormatName(format)
              << " element_size_bytes " << element_size_bytes
              << " soa " << soa
              << " mul " << mul
              << " soa*mul " << soa*mul
              << " unknown " << unknown 
              << std::endl 
              ;
    assert(expected );

    setMultiplicity(mul)  ;
    setSizeOfAtom(soa) ;
    // these do not change when buffer size changes
}


void OBufBase::setSizeOfAtom(unsigned long long soa)
{
    m_sizeofatom = soa ; 
} 
void OBufBase::setMultiplicity(unsigned long long mul)
{
    m_multiplicity = mul ; 
} 



/**
OBufBase::Size
---------------

Product of optix::Buffer width*height*depth which means that 
this size does not descend into the format (eg float4) it
is rather the number of such elements.

**/

unsigned long long OBufBase::Size(const optix::Buffer& buffer) // static
{
    RTsize width, height, depth ; 
    buffer->getSize(width, height, depth);
    RTsize size = width*height*depth ; 
    return size ; 
}

unsigned long long OBufBase::NumBytes(const optix::Buffer& buffer) // static
{
    unsigned long long size = Size(buffer);

    RTformat format = buffer->getFormat() ;
    unsigned long long element_size = OFormat::ElementSizeInBytes(format);
    if(element_size == 0ull && format == RT_FORMAT_USER)
    {
        element_size = buffer->getElementSize();
        //printf("OBufBase::getNumBytes RT_FORMAT_USER element_size %u size %u \n", element_size, size );
    }
    return size*element_size ; 
}

void OBufBase::upload(NPYBase* npy)
{
    void* data = npy->getBytes() ;
    assert(data);

    unsigned int numBytes = npy->getNumBytes(0);
    unsigned int x_numBytes = getNumBytes();
    assert(numBytes == x_numBytes);

    //printf("OBufBase::upload nbytes %u \n", numBytes);

    memcpy( m_buffer->map(), data, numBytes );

    m_buffer->unmap();
}


void OBufBase::download(NPYBase* npy)
{
    unsigned int numBytes = npy->getNumBytes(0) ;
    unsigned int x_numBytes = getNumBytes();
    assert(numBytes == x_numBytes);

    void* ptr = m_buffer->map() ; 

    npy->read( ptr );

    m_buffer->unmap(); 
}

