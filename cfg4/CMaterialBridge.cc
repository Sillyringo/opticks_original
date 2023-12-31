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

#include "BStr.hh"
#include <sstream>
#include <iomanip>



#include "G4StepPoint.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"

#include "Opticks.hh"

#include "GMaterialLib.hh"
#include "CStep.hh"
#include "CMaterialBridge.hh"

#include "PLOG.hh"


const plog::Severity CMaterialBridge::LEVEL = PLOG::EnvLevel("CMaterialBridge", "DEBUG") ; 


CMaterialBridge::CMaterialBridge(const GMaterialLib* mlib) 
    :
    m_mlib(mlib),
    m_ok(mlib->getOpticks()),
    m_test(m_ok->isTest()),
    m_mlib_materials(m_mlib->getNumMaterials()),
    m_g4_materials(G4Material::GetNumberOfMaterials())
{
    if(isValid())
    {
        initMap();
        if(LEVEL < debug) dump("CMaterialBridge::CMaterialBridge");
        // fatal:1, error:2, warning:3, info:4, debug:5, verbose:6
    }
    else
    {
         LOG(error) << "cannot boot CMaterialBridge without geant4 materials " ; 
    }

}

bool CMaterialBridge::isValid() const
{
    return m_g4_materials > 0 && m_mlib_materials > 0 ; 
}

void CMaterialBridge::initMap()
{
    const G4MaterialTable* mtab = G4Material::GetMaterialTable();
    unsigned nmat = m_g4_materials ; 
    unsigned nmat_mlib = m_mlib_materials ; 

    if( nmat == 0 )
        LOG(fatal) 
            << " THERE ARE NO Geant4 materials "
            << " nmat (G4Material::GetNumberOfMaterials) " << nmat 
            << " nmat_mlib (GMaterialLib::getNumMaterials) " << nmat_mlib 
            ; 
    assert( nmat > 0 ); 


    LOG(LEVEL)
        << " mtab " << mtab
        << " nmat (G4Material::GetNumberOfMaterials) " << nmat 
        << " nmat_mlib (GMaterialLib::getNumMaterials) " << nmat_mlib 
        ;

    std::stringstream ss ; 
    ss
        << " nmat (G4Material::GetNumberOfMaterials) " << nmat
        << " nmat_mlib (GMaterialLib::getNumMaterials) materials used by geometry " << nmat_mlib
        << std::endl
        ; 


    for(unsigned i=0 ; i < nmat ; i++)
    {
        const G4Material* material = (*mtab)[i];

        std::string name = material->GetName() ;

        const char* shortname = BStr::afterLastOrAll( name.c_str(), '/' );

        std::string abbr = shortname ; //  

        unsigned index =  m_mlib->getIndex( shortname );
        bool mlib_unset = GMaterialLib::IsUnset(index) ; 

        if(!mlib_unset)
        {
            m_g4toix[material] = index ; 
            m_ixtoname[index] = shortname ;
            m_ixtoabbr[index] = m_mlib->getAbbr(shortname) ;
        }

        ss
            << " i " << std::setw(3) << i 
            << " name " << std::setw(35) << name 
            << " shortname " << std::setw(35) << shortname 
            << " abbr " << std::setw(35) << abbr 
            << " index " << std::setw(5)  << index
            << " mlib_unset " << std::setw(5)  << mlib_unset
            << std::endl
            ; 
    }

    ss
        << " nmat " << nmat 
        << " nmat_mlib " << nmat_mlib 
        << " m_g4toix.size() "   << m_g4toix.size() 
        << " m_ixtoname.size() " << m_ixtoname.size() 
        << " m_ixtoabbr.size() " << m_ixtoabbr.size() 
        << std::endl
        ; 


    std::string s = ss.str(); 

    LOG(LEVEL) << std::endl << s  ; 

    // for matching with GDML exports the GMaterialLib has changed to handling only the 
    // materials that are used, not all G4 materials that are present 
    // so the below constraints now use nmat_mlib rather than nmat
    //
    // BUT : it seems the artifical addition of test materials doesnt fit in with this
    //

    bool mismatch = false ; 
    if( m_g4toix.size() != nmat_mlib )
    {
        mismatch = true ; 
        LOG(fatal) << " MISMATCH : m_g4toix.size() " << m_g4toix.size() << " nmat_mlib " << nmat_mlib ; 
    }

    if( m_ixtoname.size() != nmat_mlib )
    {
        mismatch = true ; 
        LOG(fatal) << " MISMATCH : m_ixtoname.size() " << m_ixtoname.size() << " nmat_mlib " << nmat_mlib ; 
    }

    if( m_ixtoabbr.size() != nmat_mlib )
    {
        mismatch = true ; 
        LOG(fatal) << " MISMATCH : m_ixtoabbr.size() " << m_ixtoabbr.size() << " nmat_mlib " << nmat_mlib ; 
    }

    if(mismatch)
    {
        LOG(error) << " MISMATCH mlib.desc " <<  m_mlib->desc(); 
    }

    if( m_test == false )
    {
        assert( m_g4toix.size() == nmat_mlib );
        assert( m_ixtoname.size() == nmat_mlib && "there is probably a duplicated material name");
        assert( m_ixtoabbr.size() == nmat_mlib && "there is probably a duplicated material name");
    }
}



void CMaterialBridge::dumpMap(const char* msg) const 
{
    LOG(info) << msg << " g4toix.size " << m_g4toix.size() ;

    typedef std::map<const G4Material*, unsigned> MU ; 
    for(MU::const_iterator it=m_g4toix.begin() ; it != m_g4toix.end() ; it++)
    {
         const G4Material* mat = it->first ; 
         unsigned index = it->second ; 

         std::cout << std::setw(50) << mat->GetName() 
                   << std::setw(10) << index 
                   << std::endl ; 

         unsigned check = getMaterialIndex(mat);
         assert(check == index);
    }
}


void CMaterialBridge::dump(const char* msg) const 
{
    LOG(info) << msg << " g4toix.size " << m_g4toix.size() ;

    typedef std::vector<const G4Material*> M ; 
    M materials ; 

    typedef std::map<const G4Material*, unsigned> MU ; 
    for(MU::const_iterator it=m_g4toix.begin() ; it != m_g4toix.end() ; it++) 
         materials.push_back(it->first);

    std::stable_sort( materials.begin(), materials.end(), *this );          

    for(M::const_iterator it=materials.begin() ; it != materials.end() ; it++)
    {
        const G4Material* mat = *it ;  
        unsigned index = getMaterialIndex(mat);
        const char* shortname = getMaterialName(index, false);
        const char* abbr = getMaterialName(index, true);

        std::cout << std::setw(50) << mat->GetName() 
                  << std::setw(10) << index 
                  << std::setw(30) << shortname 
                  << std::setw(30) << abbr
                  << std::endl ; 
    }
}


bool CMaterialBridge::operator()(const G4Material* a, const G4Material* b)
{
    unsigned ia = getMaterialIndex(a);
    unsigned ib = getMaterialIndex(b);
    return ia < ib ; 
}




unsigned CMaterialBridge::getPreMaterial(const G4Step* step) const
{
    const G4Material* preMat  = CStep::PreMaterial(step);
    unsigned preMaterial = preMat ? getMaterialIndex(preMat) + 1 : 0 ;
    return preMaterial ; 
}

unsigned CMaterialBridge::getPostMaterial(const G4Step* step) const
{
    const G4Material* postMat  = CStep::PostMaterial(step);
    unsigned postMaterial = postMat ? getMaterialIndex(postMat) + 1 : 0 ;
    return postMaterial ;
}

unsigned CMaterialBridge::getPointMaterial(const G4StepPoint* point) const
{
    const G4Material* pointMat  = point->GetMaterial() ;
    unsigned pointMaterial = pointMat ? getMaterialIndex(pointMat) + 1 : 0 ;
    return pointMaterial ;
}


unsigned int CMaterialBridge::getMaterialIndex(const G4Material* material) const 
{
    // used from CSteppingAction::UserSteppingActionOptical to CRecorder::setBoundaryStatus
    return m_g4toix.at(material) ;
}
const char* CMaterialBridge::getMaterialName(unsigned index, bool abbrev) const 
{
    return abbrev ? m_ixtoabbr.at(index).c_str() : m_ixtoname.at(index).c_str() ;
}


const G4Material* CMaterialBridge::getG4Material(unsigned int qindex) const // 0-based Opticks material index to G4Material
{
    typedef std::map<const G4Material*, unsigned> MU ; 
    const G4Material* mat = NULL ; 

    std::stringstream ss ; 

    for(MU::const_iterator it=m_g4toix.begin() ; it != m_g4toix.end() ; it++)
    {
         unsigned index = it->second ; 
         ss << index << " " ; 
         if(index == qindex)
         {
             mat = it->first ; 
             break ;
         }
    }


    std::string indices = ss.str(); 
 
    if( mat == NULL )
    {
        LOG(fatal) 
             << " failed to find a G4Material with index " << qindex 
             << " in all the indices " << indices 
             ;
    }

    return mat ; 
}


std::string CMaterialBridge::MaterialSequence(unsigned long long seqmat, bool abbrev) const 
{
    std::stringstream ss ;
    assert(sizeof(unsigned long long)*8 == 16*4);
    for(unsigned int i=0 ; i < 16 ; i++)
    {   
        unsigned long long msk = (seqmat >> i*4) & 0xF ; 

        unsigned int idx = unsigned(msk - 1);    // -> 0-based

        ss << ( msk > 0 ? getMaterialName(idx, abbrev) : "-" ) << " " ;
        // using 1-based material indices, so 0 represents None
    }   
    return ss.str();
}

