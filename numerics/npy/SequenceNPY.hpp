#pragma once

#include "glm/fwd.hpp"

#include <map>
#include <string>
#include <vector>

#include "Types.hpp"
#include "NPY.hpp"

class RecordsNPY ; 
class Index ; 

//
// precise agreement between Photon and Record histories
// demands setting a bounce max less that maxrec
// in order to avoid any truncated and top record slot overwrites 
//
// eg for maxrec 10 bounce max of 9 (option -b9) 
//    succeeds to give perfect agreement  
//                 

class SequenceNPY {
   public:  
       enum {
              e_seqhis , 
              e_seqmat 
            };
   public:  
       SequenceNPY(NPY<float>* photons); 
   public:  
       void                  setTypes(Types* types);
       void                  setRecs(RecordsNPY* recs);
   public:  
       NPY<float>*           getPhotons();
       RecordsNPY*           getRecs();
       Types*                getTypes();
   public:  
       Index*                makeHexIndex(const char* itemtype);
   public:  
       void                  indexSequences();
   public:  
       void                  dumpUniqueHistories();
       void                  countMaterials();
   public:  
       NPY<unsigned char>*   getSeqIdx();
       Index*                getSeqHis(); 
       Index*                getSeqHisHex(); 
       Index*                getSeqMat(); 

   private:
       static bool second_value_order(const std::pair<int,int>&a, const std::pair<int,int>&b);
       static bool su_second_value_order(const std::pair<std::string,unsigned int>&a, const std::pair<std::string,unsigned int>&b);

   private:
       Index* makeSequenceCountsIndex( 
               Types::Item_t etype, 
               std::map<std::string, unsigned int>& su,
               std::map<std::string, std::vector<unsigned int> >&  sv,
               unsigned int cutoff,
               bool hex=false
               );

       void fillSequenceIndex(
                unsigned int k,
                Index* idx, 
                std::map<std::string, std::vector<unsigned int> >&  sv 
                );

       void dumpMaskCounts(
                const char* msg, 
                Types::Item_t etype, 
                std::map<unsigned int, unsigned int>& uu, 
                unsigned int cutoff
                );

       void dumpSequenceCounts(
                const char* msg, 
                Types::Item_t etype, 
                std::map<std::string, unsigned int>& su,
                std::map<std::string, std::vector<unsigned int> >& sv,
                unsigned int cutoff
                );

   private:
       NPY<float>*                  m_photons ; 
       RecordsNPY*                  m_recs ; 
       Types*                       m_types ; 
       NPY<unsigned char>*          m_seqidx  ; 
       unsigned int                 m_maxrec ; 
       Index*                       m_seqhis ; 
       Index*                       m_seqhis_hex ; 
       Index*                       m_seqmat ; 

};


inline SequenceNPY::SequenceNPY(NPY<float>* photons) 
       :  
       m_photons(photons),
       m_recs(NULL),
       m_types(NULL),
       m_seqidx(NULL),
       m_maxrec(0),
       m_seqhis(NULL),
       m_seqmat(NULL)
{
}

inline void SequenceNPY::setTypes(Types* types)
{  
    m_types = types ; 
}


inline Index* SequenceNPY::getSeqHis()
{
    return m_seqhis ; 
}
inline Index* SequenceNPY::getSeqHisHex()
{
    return m_seqhis_hex ; 
}
inline Index* SequenceNPY::getSeqMat()
{
    return m_seqmat ; 
}




