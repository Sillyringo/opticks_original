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

#include <cassert>
#include <sstream>
#include <algorithm>
#include <bitset>

#include "SBit.hh"
#include "SStr.hh"
#include "SGeo.hh"
#include "SPath.hh"

#include "BFile.hh"
#include "BStr.hh"
#include "NPY.hpp"

#include "Opticks.hh"
#include "OpticksCfg.hh"
#include "OpticksDbg.hh"

#include "SLOG.hh"

const plog::Severity OpticksDbg::LEVEL = SLOG::EnvLevel("OpticksDbg", "DEBUG" ); 

OpticksDbg::OpticksDbg(Opticks* ok) 
   :
   m_ok(ok),
   m_cfg(NULL),
   m_geo(NULL),        // SGeo
   m_mask_buffer(NULL),
   m_enabledmergedmesh(0)
{
}


unsigned OpticksDbg::getNumDbgPhoton() const 
{
    return m_debug_photon.size() ; 
}
unsigned OpticksDbg::getNumOtherPhoton() const 
{
    return m_other_photon.size() ; 
}
unsigned OpticksDbg::getNumGenPhoton() const 
{
    return m_gen_photon.size() ; 
}


unsigned OpticksDbg::getNumMaskPhoton() const 
{
    return m_mask.size() ; 
}
unsigned OpticksDbg::getNumX4PolySkip() const 
{
    return m_x4polyskip.size() ; 
}
unsigned OpticksDbg::getNumX4BalanceSkip() const 
{
    return m_x4balanceskip.size() ; 
}
unsigned OpticksDbg::getNumX4NudgeSkip() const 
{
    return m_x4nudgeskip.size() ; 
}
unsigned OpticksDbg::getNumX4TubsNudgeSkip() const 
{
    return m_x4tubsnudgeskip.size() ; 
}
unsigned OpticksDbg::getNumX4PointSkip() const 
{
    return m_x4pointskip.size() ; 
}




unsigned OpticksDbg::getNumCSGSkipLV() const 
{
    return m_csgskiplv.size() ; 
}
unsigned OpticksDbg::getNumCXSkipLV() const 
{
    return m_cxskiplv.size() ; 
}

unsigned OpticksDbg::getNumDeferredCSGSkipLV() const 
{
    return m_deferredcsgskiplv.size() ; 
}






NPY<unsigned>* OpticksDbg::getMaskBuffer() const
{
    return m_mask_buffer ; 
}

unsigned OpticksDbg::getMaskIndex(unsigned idx) const 
{
    bool in_range = idx < m_mask.size() ;
    if(!in_range)
    {
         LOG(fatal) 
             << " OUT OF RANGE " 
             << " idx " << idx
             << " m_mask.size() " << m_mask.size()
             ;
    } 
    assert( in_range );
    return m_mask[idx] ; 
}


const std::vector<unsigned>&  OpticksDbg::getMask()
{
   return m_mask ;
}



void OpticksDbg::loadNPY1(std::vector<unsigned>& vec, const char* path )
{
    NPY<unsigned>* u = NPY<unsigned>::load(path) ;
    if(!u) 
    {
       LOG(warning) << " failed to load " << path ; 
       return ; 
    } 
    assert( u->hasShape(-1) );
    //u->dump();

    u->copyTo(vec);

    LOG(verbose) << "loaded " << vec.size() << " from " << path ; 
    assert( vec.size() == u->getShape(0) ); 
}



void OpticksDbg::postconfigure()
{
   LOG(verbose) << "setting up"  ; 
   m_cfg = m_ok->getCfg();

   const std::string& dindex = m_cfg->getDbgIndex() ;
   const std::string& oindex = m_cfg->getOtherIndex() ;
   const std::string& gindex = m_cfg->getGenIndex() ;

   const std::string& mask = m_cfg->getMask() ;
   const std::string& x4polyskip = m_cfg->getX4PolySkip() ;
   const std::string& x4balanceskip = m_cfg->getX4BalanceSkip() ;
   const std::string& x4nudgeskip = m_cfg->getX4NudgeSkip() ;
   const std::string& x4tubsnudgeskip = m_cfg->getX4TubsNudgeSkip() ;
   const std::string& x4pointskip = m_cfg->getX4PointSkip() ;
   const std::string& csgskiplv = m_cfg->getCSGSkipLV() ;
   const std::string& cxskiplv = m_cfg->getCXSkipLV() ;
   const std::string& deferredcsgskiplv = m_cfg->getDeferredCSGSkipLV() ;
   const std::string& enabledmm = m_cfg->getEnabledMergedMesh() ;   // default is ~0 meaning everything enabled
   const std::string& x4skipsolidname = m_cfg->getX4SkipSolidName() ;

   postconfigure( dindex, m_debug_photon );
   postconfigure( oindex, m_other_photon );
   postconfigure( gindex, m_gen_photon );

   postconfigure( mask, m_mask );
   postconfigure( x4polyskip, m_x4polyskip );
   postconfigure( x4balanceskip, m_x4balanceskip );
   postconfigure( x4nudgeskip, m_x4nudgeskip );
   postconfigure( x4tubsnudgeskip, m_x4tubsnudgeskip );
   postconfigure( x4pointskip, m_x4pointskip );
   postconfigure( csgskiplv, m_csgskiplv );
   postconfigure( cxskiplv, m_cxskiplv );
   postconfigure( deferredcsgskiplv, m_deferredcsgskiplv );
   postconfigure( enabledmm, m_enabledmergedmesh );    
   postconfigure( x4skipsolidname, m_x4skipsolidname, ',' );


   LOG(debug) << " m_csgskiplv  " << m_csgskiplv.size() ; 
   LOG(debug) << " m_deferredcsgskiplv  " << m_deferredcsgskiplv.size() ; 
   //assert(  m_csgskiplv.size() > 0 );  


   const std::string& instancemodulo = m_cfg->getInstanceModulo() ;
   postconfigure( instancemodulo,   m_instancemodulo ) ;  

   if(m_mask.size() > 0)
   {
       m_mask_buffer = NPY<unsigned>::make_from_vec(m_mask); 
   } 


   const std::string& arglist  = m_cfg->getArgList();    // --arglist 
   postconfigure( arglist, m_arglist, '\n' ); 


   LOG(debug) << "OpticksDbg::postconfigure" << description() ; 
}

/**
OpticksDbg::postgeometry
-------------------------

Invoked from Opticks::postgeometry.

**/

void OpticksDbg::postgeometry()
{
    postgeometrySkipSolidName();  
}


/**
OpticksDbg::isSkipSolidIdx
---------------------------

Canonical usage via Opticks::isSkipSolidIdx is from GInstancer::labelRepeats_r 
which 

**/
bool OpticksDbg::isSkipSolidIdx(unsigned lvIdx) const   // --skipsolidname
{
    return IsListed(lvIdx, m_skipsolididx, false); 
}


/**
OpticksDbg::postgeometrySkipSolidName
-----------------------------------------

Looks for solids within the geometry with names starting with the 
comma delimited names from the --skipsolidname argument and adds 
the midx (mesh indices) of them to m_skipsolididx
for later querying with OpticksDbg::isSkipSolidIdx.


X4 solid skipping happens much earlier in X4 conversion so cannot
rely on having a list of names to match against to convert names
into indices.  Must instead just simply compare strings. 


**/

void OpticksDbg::postgeometrySkipSolidName()
{
    LOG(LEVEL) << "[" ; 
    assert(m_cfg); 
    m_geo = m_ok->getGeo(); 
    assert(m_geo); 

    const std::string& skipsolidname = m_cfg->getSkipSolidName() ;

    std::vector<std::string> solidname ; 
    SStr::Split(skipsolidname.c_str(), ',', solidname ); 

    LOG(LEVEL) 
        << " skipsolidname " << skipsolidname 
        << " solidname.size " << solidname.size()
        ;

    std::vector<unsigned>& soidx = m_skipsolididx ; 

    unsigned notfound = 0 ; 

    for(int i=0 ; i < int(solidname.size()) ; i++)
    {
        const std::string& sn = solidname[i];  
        bool startswith = true ; 
        int midx = m_geo->getMeshIndexWithName(sn.c_str(), startswith) ; 
        bool found = midx > 0 ;    // looks like cannot find first 

        if(!found)
        {
            notfound += 1 ; 
            LOG(fatal) 
                << " failed to find solid with name starting [" << sn << "]" ; 
                ; 
        }
        else
        {
            LOG(LEVEL) 
                << " found sn [" << sn << "]" 
                << " midx "  << std::setw(4) << midx 
                ;
            soidx.push_back(midx); 
        }
    }

    LOG(notfound == 0 ? LEVEL : error) 
        << " --skipsolidname " << skipsolidname 
        << " solidname.size " << solidname.size() 
        << " soidx.size " << soidx.size()
        << " (soidx list used by Opticks::isSkipSolidIdx / GInstancer::labelRepeats_r ) "
        << " notfound " << notfound 
        ;


    LOG(LEVEL) << "]" ; 
}




void OpticksDbg::postconfigure(const std::string&  path, std::vector<std::string>& lines, char delim  )
{
    SStr::LoadList( path.c_str(), lines, delim ); 
}

void OpticksDbg::postconfigure(const std::string& spec, std::vector<std::pair<int, int> >& pairs )
{
    // "1:5,2:10" -> (1,5),(2,10)
    BStr::pair_split( pairs, spec.c_str(), ',', ":" ); 
}

/**
OpticksDbg::postconfigure bitfield
-------------------------------------

Default spec is ~0 


**/

void OpticksDbg::postconfigure(const std::string& spec, unsigned long long& bitfield)
{
    bitfield = SBit::FromString(spec.c_str()); 
    LOG(LEVEL)
        << " spec " << spec
        << " SBit::PosString(bitfield,',',true) " << SBit::PosString(bitfield,',',true)
        ;   
}


void OpticksDbg::postconfigure(const std::string& spec, std::vector<unsigned>& ls)
{
   if(spec.empty())
   {
       LOG(verbose) << "spec empty" ;
   } 
   else if(BFile::LooksLikePath(spec.c_str()))
   { 
       loadNPY1(ls, spec.c_str() );
   }
   else
   { 
       LOG(verbose) << " spec " << spec ;  
       BStr::usplit(ls, spec.c_str(), ',');
   }
}

unsigned OpticksDbg::getInstanceModulo(unsigned mm) const 
{
    unsigned size = m_instancemodulo.size() ; 
    if( size == 0u ) return 0u ; 
    typedef std::pair<int, int> II ; 
    for(unsigned i=0 ; i < size ; i++)
    {
        const II& ii = m_instancemodulo[i] ; 
        if( unsigned(ii.first) == mm ) return unsigned(ii.second) ;   
    }
    return 0u ; 
}


bool OpticksDbg::IsListed(unsigned idx, const std::vector<unsigned>& ls, bool emptylistdefault )  // static
{
    return ls.size() == 0 ? emptylistdefault : std::find(ls.begin(), ls.end(), idx ) != ls.end() ; 
}



bool OpticksDbg::isDbgPhoton(unsigned record_id) const 
{
    return IsListed(record_id, m_debug_photon,  false); 
}
bool OpticksDbg::isOtherPhoton(unsigned record_id) const 
{
    return IsListed(record_id, m_other_photon, false); 
}
bool OpticksDbg::isGenPhoton(unsigned record_id) const 
{
    return IsListed(record_id, m_gen_photon, false); 
}


bool OpticksDbg::isMaskPhoton(unsigned record_id) const 
{
    return IsListed(record_id, m_mask, false); 
}
bool OpticksDbg::isX4PolySkip(unsigned lvIdx) const 
{
    return IsListed(lvIdx, m_x4polyskip, false); 
}
bool OpticksDbg::isX4BalanceSkip(unsigned lvIdx) const 
{
    return IsListed(lvIdx, m_x4balanceskip, false); 
}
bool OpticksDbg::isX4NudgeSkip(unsigned lvIdx) const 
{
    return IsListed(lvIdx, m_x4nudgeskip, false); 
}
bool OpticksDbg::isX4TubsNudgeSkip(unsigned lvIdx) const 
{
    return IsListed(lvIdx, m_x4tubsnudgeskip, false); 
}
bool OpticksDbg::isX4PointSkip(unsigned lvIdx) const 
{
    return IsListed(lvIdx, m_x4pointskip, false); 
}



bool OpticksDbg::IsStringListed( const char* str, const std::vector<std::string>& ls, bool emptylistdefault )
{
    return ls.size() == 0 ? emptylistdefault : std::find(ls.begin(), ls.end(), str ) != ls.end() ; 
}
std::string OpticksDbg::GetList( const std::vector<unsigned>& ls, char delim ) // static
{
    unsigned num = ls.size(); 
    std::stringstream ss ; 
    for(unsigned i=0 ; i < num ; i++) 
    {
        ss << ls[i] ; 
        if( i < num-1 ) ss << delim ; 
    }
    std::string s = ss.str(); 
    return s ; 
}

bool OpticksDbg::isX4SkipSolidName(const char* soname) const 
{
    return IsStringListed(soname, m_x4skipsolidname, false); 
}


bool OpticksDbg::isCSGSkipLV(unsigned lvIdx) const   // --csgskiplv
{
    return IsListed(lvIdx, m_csgskiplv, false); 
}



bool OpticksDbg::isCXSkipLV(unsigned lvIdx) const   // --cxskiplv
{
    return IsListed(lvIdx, m_cxskiplv, false); 
}
std::string OpticksDbg::getCXSkipLVList() const  // --cxskiplv
{
    return GetList(m_cxskiplv, ','); 
}


bool OpticksDbg::isDeferredCSGSkipLV(unsigned lvIdx) const   // --deferredcsgskiplv
{
    return IsListed(lvIdx, m_deferredcsgskiplv, false); 
}

/**
OpticksDbg::isEnabledMergedMesh
---------------------------------

TODO: fix unfounded assumption of less than 64 compound solids ?
      or at least assert when there are more 

**/

bool OpticksDbg::isEnabledMergedMesh(unsigned mm) const 
{
    bool emptylistdefault = true ;   
    bool emm = true ;  
    if(mm < 64)
    {
        std::bitset<64> bs(m_enabledmergedmesh); 
        emm = bs.count() == 0 ? emptylistdefault : bs[mm] ;  
    }
    return emm ; 
    //return IsListed(mm, m_enabledmergedmesh, true ); 
}




bool OpticksDbg::isFullyEnabledMergedMesh() const 
{
    std::bitset<64> bs(m_enabledmergedmesh); 
    bool full_emm = bs.count() == 0 || bs.count() == 64  ; 
    return full_emm ; 
}




unsigned long long OpticksDbg::getEMM() const 
{
    return m_enabledmergedmesh ; 
}




const char* OpticksDbg::getEnabledMergedMesh() const 
{
   const std::string& enabledmm = m_cfg->getEnabledMergedMesh() ;
   return enabledmm.c_str();  
}


std::string OpticksDbg::description()
{
    std::stringstream ss ; 
    ss << " OpticksDbg "
       << " debug_photon "
       << " size: " << m_debug_photon.size()
       << " elem: (" << BStr::ujoin(m_debug_photon, ',') << ")" 
       << " other_photon "
       << " size: " << m_other_photon.size()
       << " elem: (" << BStr::ujoin(m_other_photon, ',') << ")" 
       << " gen_photon "
       << " size: " << m_gen_photon.size()
       << " elem: (" << BStr::ujoin(m_gen_photon, ',') << ")" 
       ;
    return ss.str(); 
}


const std::vector<unsigned>&  OpticksDbg::getDbgIndex()
{
   return m_debug_photon ;
}
const std::vector<unsigned>&  OpticksDbg::getOtherIndex()
{
   return m_other_photon ;
}
const std::vector<unsigned>&  OpticksDbg::getGenIndex()
{
   return m_gen_photon ;
}
const std::vector<std::string>& OpticksDbg::getArgList() const // --arglist
{
   return m_arglist ; 
}
