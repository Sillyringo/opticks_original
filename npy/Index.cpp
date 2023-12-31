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
#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <sstream>

// brap-
#include "BMap.hh"
#include "BStr.hh"
#include "BFile.hh"

// npy-
#include "Index.hpp"

#include "SLOG.hh"


const plog::Severity Index::LEVEL = SLOG::EnvLevel("Index", "DEBUG") ; 

int Index::COUNT = 0 ; 

Index::Index(const char* itemtype, const char* reldir, const char* title, bool onebased)
    : 
    NSequence(),
    m_itemtype(strdup(itemtype)),
    m_reldir(reldir ? strdup(reldir) : NULL),
    m_title(title ? strdup(title) : strdup(itemtype)),
    m_ext(strdup(".json")),
    m_selected(0),
    m_onebased(onebased),
    m_source_total(0),
    m_count(COUNT)
{
    LOG(LEVEL) << "ctor " << m_count  ; 
    COUNT += 1 ; 
}

Index::~Index()
{
   LOG(LEVEL) << "dtor " << m_count ; 
   free((char*)m_itemtype);
   free((char*)m_reldir);
   free((char*)m_title);
   free((char*)m_ext);
}

const char* Index::getItemType()
{
    return m_itemtype ; 
}
const char* Index::getRelDir()
{
    return m_reldir ; 
}
const char* Index::getTitle()
{
    return m_title ; 
}

bool Index::isOneBased()
{
    return m_onebased ; 
}



void Index::setTitle(const char* title)
{
    if(!title) return ;
    free((void*)m_title);
    m_title = strdup(title); 
}


std::vector<std::string>& Index::getNames()
{
    return m_names ;  
}

void Index::setExt(const char* ext)
{   
    m_ext = strdup(ext);
}


int Index::getSelected()
{
    return m_selected ; 
}
int* Index::getSelectedPtr()
{
    return &m_selected ; 
}
const char* Index::getSelectedKey()
{
    return getKey(m_selected);
}


std::string Index::description()
{
    std::stringstream ss ; 

    const char* type = getItemType() ;
    const char* title = getTitle() ;

    ss << "Index" 
       << " itemtype " << ( type ? type : "NULL" )
       << " title " << ( title ? title : "NULL" )
       << " numItems " << getNumItems()
       ;

    return ss.str();
}



void Index::add(const VS& vs)
{
    unsigned int n(0);
    for(VS::const_iterator it=vs.begin(); it!=vs.end() ; it++) 
    {
        add(it->c_str(),n+1);
        n++ ;
    }
}

/**
Index::add
------------

only the first ocurrence of a repeated name is added
local index incremented for each unique name

**/
void Index::add(const char* name, unsigned source, bool sort )
{
    LOG(LEVEL) << "[" ; 
    if(m_source.count(name)==0)
    { 
        m_source[name] = source ;

       // historically have been using : 1-based index in addition order  
        unsigned local = m_onebased ? m_local.size() + 1 : m_local.size() ; 
        m_local[name] = local ; 

        m_source2local[source] = local ; 
        m_local2source[local]  = source ; 
   
        if(sort) sortNames(); // when dealing with very big indices could just do this after all adds are made 
    }
    LOG(LEVEL) << "]" ; 
}

/**
Index::sortNames
-----------------

Former approach with operator() and "*this" was calling dtor on every call 
so no go unless just leak::

   std::sort(m_names.begin(), m_names.end(), *this ); // ascending local index

The reason is explained by the below link. 

* https://stackoverflow.com/questions/13384576/c-stdsort-calling-destructor


**/

void Index::sortNames()
{
   typedef std::map<std::string, unsigned int> MSU ; 
   m_names.clear();
   for(MSU::iterator it=m_local.begin() ; it != m_local.end() ; it++ ) m_names.push_back(it->first) ;


   std::sort(m_names.begin(), m_names.end(), std::bind(&Index::local_order, this, std::placeholders::_1, std::placeholders::_2));
}

bool Index::local_order(const std::string& a, const std::string& b)
{
    return m_local[a] < m_local[b] ; 
}

unsigned int Index::getIndexLocal(const char* name, unsigned int missing) const 
{
    return m_local.count(name) == 1 ? m_local.at(name) : missing ; 
}

bool Index::hasItem(const char* name)
{
    return m_local.count(name) == 1 ; 
}


unsigned int Index::getNumItems()
{
    //assert(m_source.size() == m_local.size());
    return m_local.size();
}


// fulfil NSequence
//   NSequence indices are zero-based, so have to convert when one-based
unsigned int Index::getNumKeys() const 
{
    return m_local.size();
}
const char* Index::getKey(unsigned int i) const 
{
    unsigned int local = m_onebased ? i + 1 : i ;
    return getNameLocal(local);
}
unsigned int Index::getIndex(const char* key) const
{
    unsigned int local = getIndexLocal(key);
    return m_onebased ? local - 1 : local ; 
}


unsigned int Index::getIndexSourceTotal()
{
    if(m_source_total == 0)
    {
        typedef std::map<std::string, unsigned int> MSU ; 
        for(MSU::iterator it=m_source.begin() ; it != m_source.end() ; it++ ) m_source_total += it->second ; 
    }
    return m_source_total ; 
}

float Index::getIndexSourceFraction(const char* name)
{
     unsigned int total = getIndexSourceTotal();
     unsigned int value = getIndexSource(name);
     return total > 0 ? float(value)/float(total) : 0.f ; 
}

unsigned int Index::getIndexSource(const char* name, unsigned int missing)
{
    return m_source.count(name) == 1 ? m_source[name] : missing ; 
}

unsigned Index::getIndexSourceStarting(const char* name, unsigned int missing)
{
    typedef std::map<std::string, unsigned int> MSU ; 
    unsigned source_idx = missing ;  
    for(MSU::const_iterator it=m_source.begin() ; it != m_source.end() ; it++ ) 
    {
         std::string it_name = it->first ; 
         unsigned it_idx = it->second ; 
         if(BStr::StartsWith(it_name.c_str(), name))
         {
             source_idx = it_idx ; 
             break ; 
         } 
    } 
    return source_idx ; 
}



const char* Index::getNameLocal(unsigned int local, const char* missing) const 
{
    typedef std::map<std::string, unsigned int> MSU ; 
    for(MSU::const_iterator it=m_local.begin() ; it != m_local.end() ; it++ ) 
        if(it->second == local) return it->first.c_str();
    return missing ; 
}
const char* Index::getNameSource(unsigned int source, const char* missing) const 
{
    typedef std::map<std::string, unsigned int> MSU ; 
    for(MSU::const_iterator it=m_source.begin() ; it != m_source.end() ; it++ ) 
        if(it->second == source) return it->first.c_str();
    return missing ; 
}


unsigned int Index::convertSourceToLocal(unsigned int source, unsigned int missing)
{
    return m_source2local.count(source) == 1 ? m_source2local[source] : missing ; 
}

unsigned int Index::convertLocalToSource(unsigned int local, unsigned int missing)
{
    return m_local2source.count(local) == 1 ? m_local2source[local] : missing ; 
}


void Index::test(const char* msg, bool verbose)
{
   LOG(info) << msg << " itemtype: " << m_itemtype  ; 

   typedef std::vector<std::string> VS ; 
   for(VS::iterator it=m_names.begin() ; it != m_names.end() ; it++ )
   {
       std::string iname = *it ; 
       unsigned int local  = m_local[iname];
       unsigned int source = m_source[iname];

       assert(strcmp(getNameLocal(local),iname.c_str())==0); 

       //assert(strcmp(getNameSource(source),iname.c_str())==0); 
       if(strcmp(getNameSource(source),iname.c_str())!=0) 
       {
           LOG(warning) << "Index::test inconsistency " 
                        << " source " << source 
                        << " iname " << iname 
                        ;
       }

       assert(getIndexLocal(iname.c_str())==local); 
       assert(getIndexSource(iname.c_str())==source); 

       //assert(convertSourceToLocal(source)==local); 
       if(convertSourceToLocal(source)!=local)
       {
           LOG(warning) << "Index::test convertSourceToLocal inconsistency " 
                        << " source " << source 
                        << " local " << local 
                        ;
       }

       //assert(convertLocalToSource(local)==source); 
       if(convertLocalToSource(local)!=source) 
       {
           LOG(warning) << "Index::test convertLocalToSource inconsistency " 
                        << " source " << source 
                        << " local " << local 
                        ;
       } 


       if(verbose) std::cout 
            << " name   " << std::setw(35) <<  iname
            << " source " << std::setw(10) <<  std::dec << source
            << " local  " << std::setw(10) <<  std::dec << local
            << std::endl ; 

   }
}

void Index::dump(const char* msg)
{
    test(msg, true);
}

void Index::crossreference()
{
   typedef std::vector<std::string> VS ; 
   for(VS::iterator it=m_names.begin() ; it != m_names.end() ; it++ )
   {
       std::string k = *it ; 
       unsigned int source = m_source[k];
       unsigned int local  = m_local[k];

       m_source2local[source] = local ; 
       m_local2source[local]  = source ; 
   }
}

/*
std::string Index::directory(const char* pfold, const char* rfold)
{
    std::stringstream ss ; 
    ss << pfold << "/" << rfold ; 
    std::string dir = ss.str();
    return dir ; 
}

void Index::save(const char* pfold, const char* rfold)
{
   std::string dir = directory(pfold, rfold);
   save(dir.c_str());
}
*/

void Index::save(const char* idpath)
{
    LOG(LEVEL) << "[ " << idpath  ;

    std::string dir = BFile::FormPath(idpath, m_reldir) ; 
    std::string sname = getPrefixedString("Source") ;
    std::string lname = getPrefixedString("Local") ;

    LOG(LEVEL) 
              << " sname " << sname 
              << " lname " << lname 
              << " itemtype " << m_itemtype
              << " ext " << m_ext 
              << " idpath " << idpath 
              << " dir " << dir 
              ;

    BMap<std::string, unsigned int>::save( &m_source, dir.c_str(), sname.c_str() );  
    BMap<std::string, unsigned int>::save( &m_local , dir.c_str(), lname.c_str() );  

    LOG(LEVEL) << "]" ;

}
std::string Index::getPrefixedString(const char* tail)
{
    std::string prefix(m_itemtype); 
    return prefix + tail + m_ext ; 
}
std::string Index::getPath(const char* idpath, const char* prefix, bool create_idpath_dir)
{
    LOG(LEVEL)
        << " idpath " << idpath
        << " prefix " << prefix
        << " create_idpath_dir " << create_idpath_dir
        ;

    std::string dir = BFile::FormPath(idpath, m_reldir) ; 
    std::string path = BFile::preparePath(dir.c_str(), getPrefixedString(prefix).c_str(), create_idpath_dir);
    return path;
}
bool Index::exists(const char* idpath)
{
    if(!idpath) return false ;
    std::string dir = BFile::FormPath(idpath, m_reldir) ; 
    bool sx = BFile::ExistsFile(dir.c_str(), getPrefixedString("Source").c_str());
    bool lx = BFile::ExistsFile(dir.c_str(), getPrefixedString("Local").c_str());
    return sx && lx ; 
}

void Index::loadMaps(const char* idpath)
{
    std::string dir = BFile::FormPath(idpath, m_reldir) ; 
    BMap<std::string, unsigned int>::load( &m_source, dir.c_str(), getPrefixedString("Source").c_str() );  
    BMap<std::string, unsigned int>::load( &m_local , dir.c_str(), getPrefixedString("Local").c_str() );  

    sortNames();
    crossreference();
}

/*
Index* Index::load(const char* pfold, const char* rfold, const char* itemtype)
{
   std::string dir = directory(pfold, rfold);
   return load(dir.c_str(), itemtype );
}
*/


void Index::dumpPaths(const char* idpath, const char* msg)
{
    std::string dir = BFile::FormPath(idpath, m_reldir) ; 
    bool sx = BFile::ExistsFile(dir.c_str(), getPrefixedString("Source").c_str());
    bool lx = BFile::ExistsFile(dir.c_str(), getPrefixedString("Local").c_str());

    bool createdir = false ; 

    LOG(info) << msg ;
    LOG(info) 
              << " Source:" << ( sx ? "EXISTS " : "MISSING" ) << getPath(idpath, "Source", createdir)
              << "  Local:" << ( lx ? "EXISTS " : "MISSING" ) << getPath(idpath, "Local",  createdir)
              ;
}


Index* Index::load(const char* idpath, const char* itemtype, const char* reldir)
{
    Index* idx = new Index(itemtype, reldir);
    if(idx->exists(idpath))
    {
       idx->loadMaps(idpath);
    }
    else
    {
        bool createdir = false ; 
        LOG(error)
            << "FAILED to load index " 
            << " idpath " << ( idpath ? idpath : "NULL" )
            << " itemtype " << itemtype 
            << " Source path " << idx->getPath(idpath, "Source", createdir)
            << " Local path " << idx->getPath(idpath, "Local", createdir)
            ;
        idx = NULL ;
    }
    return idx ; 
}


