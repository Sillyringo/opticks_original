#pragma once
// High level view of OpIndexer
//
//     indexes history and material sequences from the sequence buffer
//     (each photon has unsigned long long 64-bit ints containg step-by-step flags and material indices)   
//     The sequences are sparse histogrammed (essentially an index of the popularity of each sequence)
//     creating a small lookup table that is then applied to all photons to create the phosel and 
//     recsel arrays containg the popularity index.
//     This index allows fast selection of all photons from the top 32 categories, this
//     can be used both graphically and in analysis.
//     The recsel buffer repeats the phosel values maxrec times to provide fast access to the
//     selection at record level.
//
//
// optixwrap-/OBuf 
//       optix buffer wrapper
// 
// thrustrap-/TBuf 
//       thrust buffer wrapper, with download to NPY interface 
//
// thrustrap-/TSparse
//       GPU Thrust sparse histogramming 
//
// cudawrap-/CBufSpec   
//       lightweight struct holding device pointer
//
// cudawrap-/CBufSlice  
//       lightweight struct holding device pointer and slice addressing begin/end
//
// cudawrap-/CResource  
//       OpenGL buffer made available as a CUDA Resource, 
//       mapGLToCUDA returns CBufSpec struct 
//
// OBuf and TBuf have slice methods that return CBufSlice structs 
// identifying a view of the GPU buffers 
//
//
// NB hostside allocation deferred for these
// note the layering here, this pattern will hopefully facilitate moving 
// from OpenGL backed to OptiX backed for COMPUTE mode
// although this isnt a good example as indexSequence is not 
// necessary in COMPUTE mode 
//
// CAUTION: the lookup resides in CUDA constant memory, not in the TSparse 
// object so must apply the lookup before making another 


#include <cstddef>
class OBuf ; 
class OContext ; 
class OPropagator ; 

class TBuf ; 

class NumpyEvt ; 
struct CBufSlice ; 
struct CBufSpec ; 
template <typename T> class NPY ;
template <typename T> class TSparse ;

class Timer ; 

class OpIndexer {
   public:
      OpIndexer(OContext* ocontext);
      void setEvt(NumpyEvt* evt);
      void setNumPhotons(unsigned int num_photons);
      void setSeq(OBuf* seq);
      void setPropagator(OPropagator* propagator);
   public:
      void indexSequence(); 
   private:
      void indexSequenceLoaded();  
      void indexSequenceInterop();  
      void indexSequenceCompute();  
   private:
      void init();
      void update();
   private:
      // implemented in OpIndexer_.cu for nvcc compilation
      void indexSequenceViaThrust(         
           TSparse<unsigned long long>& seqhis, 
           TSparse<unsigned long long>& seqmat, 
           bool verbose 
      );
      void indexSequenceViaOpenGL(
           TSparse<unsigned long long>& seqhis, 
           TSparse<unsigned long long>& seqmat, 
           bool verbose 
      );


      void indexSequenceImp(
           TSparse<unsigned long long>& seqhis, 
           TSparse<unsigned long long>& seqmat, 
           const CBufSpec& rps,
           const CBufSpec& rrs,
           bool verbose 
      );
   private:

      void dump(const TBuf& tphosel, const TBuf& trecsel);
      void dumpHis(const TBuf& tphosel, const TSparse<unsigned long long>& seqhis) ;
      void dumpMat(const TBuf& tphosel, const TSparse<unsigned long long>& seqhis) ;
      void saveSel();
   private:
      // resident
      OContext*                m_ocontext ;
   private:
      // externally set 
      OPropagator*             m_propagator ; 
      OBuf*                    m_seq ; 
      NumpyEvt*                m_evt ;
   private:
      // transients updated by updateEvt at indexSequence
      NPY<unsigned char>*      m_phosel ;
      NPY<unsigned char>*      m_recsel ;
      unsigned int             m_maxrec ; 
      unsigned int             m_num_photons ; 

};

inline OpIndexer::OpIndexer(OContext* ocontext)  
   :
     m_ocontext(ocontext),
     m_propagator(NULL),
     m_seq(NULL),
     m_evt(NULL),
     m_phosel(NULL),
     m_recsel(NULL),
     m_maxrec(0),
     m_num_photons(0)
{
}

inline void OpIndexer::setSeq(OBuf* seq)
{
    m_seq = seq ; 
}
inline void OpIndexer::setPropagator(OPropagator* propagator)
{
    m_propagator = propagator ; 
}
inline void OpIndexer::setEvt(NumpyEvt* evt)
{
    m_evt = evt ; 
}



