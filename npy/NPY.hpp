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

#pragma once

#include <vector>
#include <string>
#include <set>
#include <map>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include "plog/Severity.h"


#include "NPY_API_EXPORT.hh"
#include "NPY_FLAGS.hh"

#include "uif.h"
#include "ucharfour.h"
#include "charfour.h"
#include "numpy.hpp"

#include "NBufferSpec.hpp"

#include "NPYBase.hpp"
#include "NPart.hpp"
#include "NQuad.hpp"

struct BBufSpec ; 
struct NSlice ; 
struct nmat4pair ; 
struct nmat4triple ; 
struct NP ; 

template <typename T> struct nmat4triple_ ; 


class NPYSpec ; 
class G4StepNPY ; 


/**


NPY : C++ NumPy style array 
==============================

*NPY* provides array operations includes persistency, the structure 
and approach are inspired by the NumPy array http://www.numpy.org 
and the **NPY** persistency format is adopted allowing Opticks
geometry and event data to be loaded into python OR ipython sessions
with::

   import numpy as np
   a = np.load("/path/to/file.npy")


Interop NumPy -> NPY
-----------------------

The type of the NumPy array saved from python
needs to match the NPY basis type 
eg NPY<float> NPY<short> NPY<double>

From python control the type, eg to save as float with::

    In [3]: a.dtype
    Out[3]: dtype('float64')

    In [4]: b = np.array(a, dtype=np.float32)

    np.save("/tmp/slowcomponent.npy", b ) 


Going the other way, NPY -> NumPy just works as NumPy 
recognises the type on loading, thanks to the numpy.hpp metadata header.


**/



template <class T>
class NPY_API NPY : public NPYBase {

   friend class SeqNPY ; 
   friend class AxisNPY ; 
   friend class SequenceNPY ; 
   friend class RecordsNPY ; 
   friend class PhotonsNPY ; 
   //friend class HitsNPY ; 
   friend class G4StepNPY ; 
   friend class MaterialLibNPY ; 
   friend struct test_dynamic_random ;  

   public:
        static const plog::Severity LEVEL ; 
        static Type_t type ;  // for type branching 
        static T UNSET ; 
        static bool IsReal(); 
        static bool IsInteger(); 
        static bool IsUnsigned(); 
        static bool IsChar(); 
   public:
       // NB favor vec4 over vec3 for better GPU performance (due to memory coalescing/alignment)
       static NPY<T>* make_vec3(float* m2w, unsigned int npo=100);  

       static NPY<T>* make(unsigned int ni, const NPYSpec* argspec);   // argspec ni is ignored and is replaced by ni 
       static NPY<T>* make(const NPYSpec* argspec);



       static NPY<T>* make(const std::vector<int>& shape);
       static NPY<T>* make(unsigned int ni);
       static NPY<T>* make(unsigned int ni, unsigned int nj );
       static NPY<T>* make(unsigned int ni, unsigned int nj, unsigned int nk );
       static NPY<T>* make(unsigned int ni, unsigned int nj, unsigned int nk, unsigned int nl );
       static NPY<T>* make(unsigned int ni, unsigned int nj, unsigned int nk, unsigned int nl, unsigned int nm);

       static NPY<T>* nasty_old_concat(const std::vector<const NPYBase*>& comps); 
       static NPY<T>* old_concat(const std::vector<const NPYBase*>& comps); 
       static NPY<T>* concat(const std::vector<const NPYBase*>& comps); 

       static NPY<float>*  MakeFloat(  const NPY<T>* src ); 
       static NPY<double>* MakeDouble( const NPY<T>* src ); 

       static NPY<T>* make_modulo(NPY<T>* src, unsigned int scaledown);
       static NPY<T>* make_repeat(NPY<T>* src, int n);
       static NPY<T>* make_inverted_transforms(NPY<T>* src, bool transpose=false);
       static NPY<T>* make_paired_transforms(NPY<T>* src, bool transpose=false);
       static NPY<T>* make_triple_transforms(NPY<T>* src);
       static NPY<T>* make_identity_transforms(unsigned ni);
       static NPY<T>* make_identity_transforms(unsigned ni, unsigned nj);
       static NPY<T>* make(const std::vector<glm::vec4>& vals);
       //static NPY<T>* make(const std::vector<glm::mat4>& mats);

       static NPY<T>* make_from_vec(const std::vector<T>& vals);
       static NPY<T>* make_from_str(const char* s, char delim=',');

       static NPY<T>* make_like(const NPY<T>* src);      // same shape as source, zeroed
       static NPY<T>* make_dbg_like(NPY<T>* src, int label_=0);  // same shape as source, values based on indices controlled with label_

   public:
       //static bool hasSameItemSize(NPY<T>* a, NPY<T>* b) ;   // moved to base
   public:
        static NPY<T>* make_masked(NPY<T>* src, NPY<unsigned>* msk );
        static unsigned _copy_masked(NPY<T>* dst, NPY<T>* src, NPY<unsigned>* msk );
   public:
       static NPY<T>* make_selection(NPY<T>* src, unsigned jj, unsigned kk, unsigned mask );
       static unsigned count_selection(NPY<T>* src, unsigned jj, unsigned kk, unsigned mask );
       static unsigned copy_selection(NPY<T>* dst, NPY<T>* src, unsigned jj, unsigned kk, unsigned mask );
       unsigned write_selection(NPY<T>* dst, unsigned jj, unsigned kk, unsigned mask);
  private:
       static unsigned _copy_selection(NPY<T>* dst, NPY<T>* src, unsigned jj, unsigned kk, unsigned mask );
   public:
       static unsigned modulo_count(unsigned ni, unsigned modulo, unsigned index); 
       static NPY<T>* make_modulo_selection(const NPY<T>* src, unsigned modulo, unsigned index); 
       static NPY<T>* make_interleaved( const std::vector<NPYBase*>& srcs ); 
   public:
       static std::string compare_diff( const T a, const T b, const T epsilon,  char mode  ); 
       static bool     compare_value( const T a, const T b, const T epsilon,  char mode  ); 
       static unsigned compare( const NPY<T>* a, const NPY<T>* b, const std::vector<T>&  epsilons,  bool dump, unsigned dumplimit, char mode ); 
       static unsigned compare( const NPY<T>* a, const NPY<T>* b, const T epsilon, bool dump, unsigned dumplimit, char mode, std::string* desc  ); 
       static unsigned compare_element_jk(const NPY<T>* a, const NPY<T>* b, int j, int k, bool dump );
   public:
       // ctor takes ownership of a copy of the inputs 
       NPY(const std::vector<int>& shape, const T*  data            , std::string& metadata) ;
       NPY(const std::vector<int>& shape, const std::vector<T>& data, std::string& metadata) ;

   public:
       // to allow binary level access to NPY data from for example gltf tools
       NBufferSpec getBufferSpec() const ;
   private:
       std::size_t getBufferSize(bool header_only, bool fortran_order) const ;
   public:
       static NPY<T>* debugload(const char* path);
       static NPY<T>* load(const char* path, bool quietly=false);
       static NPY<T>* load(const char* dir, const char* name, bool quietly=false);
       //static NPY<T>* loadPPM(const char* path);
 
       void save(const char* path) const ;
       void save(const char* dir, const char* name) const ;
       void save(const char* dir, const char* reldir, const char* name) const ;

       NBufferSpec saveToBuffer(std::vector<unsigned char>& vdst) const ;          // including the NPY header
       static NPY<T>* loadFromBuffer(const std::vector<unsigned char>& vsrc); // buffer must include NPY header 

       bool exists(const char* path) const ;
       bool exists(const char* dir, const char* name) const ;
   public:
       // via BOpticksEvent
       void save(          const char* pfx, const char* tfmt, const char* targ, const char* tag, const char* det) const ;
       static NPY<T>* load(const char* pfx, const char* tfmt, const char* targ, const char* tag, const char* det, bool quietly=false); 
       bool exists(        const char* pfx, const char* tfmt, const char* targ, const char* tag, const char* det ) const ;
    public:
       // between old and new array types
       NP* spawn() const ; 
       static NP* copy_(const NPY<T>* src) ;
    public:
       NPY<T>* clone() const ;
       static NPY<T>* copy(const NPY<T>* src) ;
    public:
       NPY<T>* make_slice(const char* slice);
       NPY<T>* make_slice(NSlice* slice);
   public:
       NPY<T>* transform(glm::mat4& tr);
       NPY<T>* scale(float factor);
   public:
       // property shape is (n,2) where n > 1 
       bool is_pshaped() const ; 
       void pscale(T scale, unsigned column);
       void pdump(const char* msg="NPY::pdump") const; 
   public:
       bool equals(const NPY<T>* other, bool dump=false) const ;
       T maxdiff(const NPY<T>* other, bool dump=false) const ;
   public:
       //unsigned int getNumValues(); tis in base class
       T* getValues();
       const T* getValuesConst() const ;
       T* begin();
       T* end();
       T*       getValues(unsigned i, unsigned j=0, unsigned k=0);
       const T* getValuesConst(unsigned i, unsigned j=0, unsigned k=0) const ;


       void* getBytes() const ;
       void* getPointer();   // aping GBuffer for easier migration
       BBufSpec* getBufSpec();

       void read(const void* src);
       void write(void* dst) const ;
       void writeItem(void* dst, unsigned item);
    private:
       T* grow(unsigned items); // increase size to contain an extra nitems, return pointer to start of them
   public:
       void reserve(unsigned items);  // set capacity to hold items  -> private ?
    public:
       unsigned expand(unsigned extra_items);  // expand buffer size to hold additional extra_items, returns new total   
       void add(const NPY<T>* other);   // add another buffer, it must have same itemsize (ie size after 1st dimension)
       void add(const T* values, unsigned int nvals);   // add values, nvals must be integral multiple of the itemsize  
       void addString(const char* s );   // add string, assumes a char array, strings truncated to size of last dimension  
       void add(void* bytes, unsigned int nbytes); // add bytes,  nbytes must be integral multiple of itemsize in bytes
       void add(T x, T y, T z, T w) ;   // add values of a quad, itemsize must be 4 
       void add(const glm::vec4& v ) ;  // add quad, itemsize must be 4 
       void add(const glm::vec4& v0, const glm::vec4& v1) ;  // add two quads, itemshape must be 2,4   

       void add(const glm::uvec4& u ) ; // add quad, itemsize must be 4 
       void add(const glm::ivec4& u ) ; // add quad, itemsize must be 4 
       void add(const glm::mat4& m ) ;  // add mat4, itemsize must be 4,4
       void reset();   //  clears data, setHasData to false and setNumItems to zero
       unsigned capacity() const ; 
    public:
       void addOffset(int j, int k, unsigned offset, bool preserve_zero=true, bool preserve_signbit=true ); 
    public:
       void updateDigests();
       void addItem(NPY<T>* other, unsigned item);   // add single item from another buffer, it must have same itemsize (ie size after 1st dimension)
       unsigned addItemUnique(NPY<T>* other, unsigned item ); 
       // Add single item from other buffer only if the item is not already present in this buffer, 
       // 
       // * the other buffer must have the same itemsize as this buffer
       // * the other buffer may of course only have a single item, in which case item will be 0 
       //
       // returns the 0-based index of the newly added item if unique or the preexisting 
       // item if not unique
       //
    public:
       std::vector<T>& data();  // TODO: replace this with vector()
       std::vector<T>& vector();

       void setData(const T* data);
       T* fill(T value);
       void fillIndexFlat(T offset=0); // fill with flattened index values, for debugging 
       int compareWithIndexFlat();  // return number of mismatches to flattened index values
       void zero();
       T* allocate();
    private:
       void deallocate();  // clears data, setHasData to false and setNumItems to zero
    public:
       T* getUnsetItem();
       bool isUnsetItem(unsigned int i, unsigned int j);
    public:
       void dump(const char* msg="NPY::dump", unsigned int limit=15);
       void minmax(T& mi, T& mx) const ;
       bool isConstant(T val) const ;
    public:
       void minmax(std::vector<T>& mi, std::vector<T>& mx) const ;
       void minmax_strided(T& mi, T& mx, unsigned stride, unsigned offset) const ;
       void minmax3(ntvec3<T>& mi_, ntvec3<T>& mx_) const ;
       void minmax4(ntvec4<T>& mi_, ntvec4<T>& mx_) const ;
       ntrange3<T> minmax3() const ;
       ntrange4<T> minmax4() const ;
    public:
       void qdump(const char* msg="NPY::qdump");
    public:
       // methods assuming 3D shape
       std::set<int> uniquei(unsigned int j, unsigned int k);

       std::map<int,int> count_uniquei(unsigned int j, unsigned int k, int sj=-1, int sk=-1);
       // when both sj and sk are >-1 the float specified is used 
       // to sign the boundary code used in the map 

       std::map<unsigned int,unsigned int> count_unique_u(unsigned int j, unsigned int k);

       std::vector<std::pair<int,int> > count_uniquei_descending(unsigned int j, unsigned int k); 
       static bool second_value_order(const std::pair<int,int>&a, const std::pair<int,int>&b);

    public:
       // type shifting get/set using union trick

       unsigned getUSum(unsigned int j, unsigned int k) const ;

       T            getValueFlat(unsigned idx) const ;
       T            getValue( int i,  int j,  int k,  int l=0, int m=0) const ;
       float        getFloat( int i,  int j,  int k,  int l=0 ) const ;

        // -ve indices are relative to ni, nj, nk, nl  
       bool         getUSign(   int i,  int j,  int k,  int l=0) const ;
       unsigned     getUInt(    int i,  int j,  int k,  int l=0) const ;
       int          getInt(     int i,  int j,  int k,  int l=0) const ;

       void         getU( short& value, unsigned short& uvalue, unsigned char& msb, unsigned char& lsb, unsigned int i, unsigned int j, unsigned int k, unsigned int l=0);

       ucharfour    getUChar4( unsigned int i, unsigned int j, unsigned int k, unsigned int l0, unsigned int l1 );
       charfour     getChar4( unsigned int i, unsigned int j, unsigned int k, unsigned int l0, unsigned int l1 );

       void         setValueFlat(unsigned idx, T value);
       void         setValue( int i,  int j,  int k,  int l, T value);
       void         setFloat( int i,  int j,  int k,  int l, float value);

       void         setAllValue( int j,  int k,  int l, T value);



        // -ve indices are relative to ni, nj, nk, nl  
       void         setUSign(int i,  int j,  int k,  int l,  bool      value);
       void         setUInt( int i,  int j,  int k,  int l,  unsigned  value);
       void         setInt(  int i,  int j,  int k,  int l,  int       value);


       void         bitwiseOrUInt( unsigned int i, unsigned int j, unsigned int k, unsigned int l, unsigned int value);


       ///  quad setters 

       void         setQuad(unsigned i, unsigned j,             T x, T y=0., T z=0., T w=0. );
       void         setQuad(unsigned i, unsigned j, unsigned k, T x, T y=0., T z=0., T w=0. );

       void         setQuad(      const nvec4& vec, unsigned int i, unsigned int j=0, unsigned int k=0 );
       void         setQuad(const   glm::vec4& vec, unsigned int i, unsigned int j=0, unsigned int k=0 );
       void         setQuad(const  glm::ivec4& vec, unsigned int i, unsigned int j=0, unsigned int k=0 );
       void         setQuad(const  glm::uvec4& vec, unsigned int i, unsigned int j=0, unsigned int k=0 );

#ifndef __CUDACC__
       // exclude for thrustrap compilation as nvcc and glm dont get along well
       void          setQuad_(const glm::tvec4<T>& vec, unsigned int i, unsigned int j=0, unsigned int k=0 );
       glm::tvec4<T> getQuad_(unsigned int i,  unsigned int j=0, unsigned int k=0 ) const ;
#endif


       /// union type shifting setters
       void         setQuadI(const glm::ivec4& vec,  int i,  int j=0,  int k=0 );
       void         setQuadI(const     nivec4& vec,  int i,  int j=0,  int k=0 );
       void         setQuadU(const glm::uvec4& vec,  int i,  int j=0,  int k=0 );
       void         setQuadU(const     nuvec4& vec,  int i,  int j=0,  int k=0 );

       void         setPart( const npart& part, unsigned int i ); ///  parts are comprised of four quads


       // 
       nvec4        getVQuad(unsigned int i,  unsigned int j=0, unsigned int k=0 ) const ;

       ///  quad getters  : NB when no union type shifting is needed its best to use the templated vec getter : getQuad_()
       glm::vec4    getQuadF( int i,  int j=0,  int k=0 ) const ;
       glm::ivec4   getQuadI( int i,  int j=0,  int k=0 ) const ;
       glm::uvec4   getQuadU( int i,  int j=0,  int k=0 ) const ;

       // Mat4 
       void         setMat4( const glm::mat4& mat, int i, int j_=-1 , bool transpose=false );
       glm::mat4    getMat4(int i, int j=-1) const ;
       glm::mat4*   getMat4Ptr(int i, int j=-1) const ;

       glm::tmat4x4<T> getMat4_(int i, int j=-1) const ;


       nmat4pair*   getMat4PairPtr(int i) const ;
       void         setMat4Pair(const nmat4pair* mpair, unsigned i );
       nmat4triple* getMat4TriplePtr(int i) const ;
       void         setMat4Triple(const nmat4triple* mpair, unsigned i );


       //nmat4triple_<T>* getMat4Triple_Ptr(int i) const ;


       void         setString( const char* s , unsigned i, unsigned j=0, unsigned k=0 );
       const char*  getString( unsigned i, unsigned j=0, unsigned k=0 );

       void         copyTo(std::vector<glm::ivec4>& dst );
       void         copyTo(std::vector<glm::vec3>& dst );
       void         copyTo(std::vector<glm::vec4>& dst );

       void         copyTo(std::vector<T>& dst );

   public:
       // interpolation of property shaped (ni,2) arrays 
       T            interp(T x) const ; 
   public:
       // Msk is used to keep note of the mask applied to an 
       // array created with *make_masked*. This enables the   
       // original indices to remain available within the masked array.

       void            setMsk(NPY<unsigned>* msk);
       NPY<unsigned>*  getMsk() const ;
       int             getMskIndex(unsigned i) const ;
       bool            hasMsk() const ; 


   //private:
   public:
       std::vector<T>     m_data ; 
       T*                 m_unset_item ; 
       BBufSpec*          m_bufspec ; 
       NPY<unsigned>*     m_msk ; 

       std::vector<std::string> m_digests ;  // usually empty, only used by addItemUnique 

 
};





