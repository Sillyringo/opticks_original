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


#include <cstdio>
#include <iostream>
#include <iomanip>
#include <string>

#include "SSys.hh"

#include "BFile.hh"
#include "BStr.hh"
#include "BEnv.hh"
#include "Map.hh"

#include "SLOG.hh"

const plog::Severity BEnv::LEVEL = SLOG::EnvLevel("BEnv","DEBUG"); 


#ifdef _MSC_VER
#else
#include <unistd.h>
extern char **environ;
#endif

BEnv* BEnv::Load(const char* dir, const char* name) 
{
    BEnv* e = new BEnv ;  
    e->readFile(dir, name);
    return e ; 
}

BEnv* BEnv::Load(const char* path) 
{
    BEnv* e = new BEnv ;  
    e->readFile(path);
    return e ; 
}

BEnv* BEnv::Create(const char* prefix)
{
    BEnv* e = new BEnv(environ); 
    e->setPrefix(prefix); 
    //e->dump("BEnv::Create"); 
    return e ; 
}



BEnv::BEnv(char** envp) 
   :  
    m_envp(envp), 
    m_prefix(NULL), 
    m_path(NULL), 
    m_all(NULL),
    m_selection(NULL)

{
   init();
   readEnv();
}

void BEnv::init()  
{
}

void BEnv::readEnv()
{
    m_all = new MSS ;
    if(!m_envp) return ; 

    while(*m_envp)
    {
       std::string kv = *m_envp++ ; 
       const size_t pos = kv.find("=") ;
       if(pos == std::string::npos) continue ;     
       std::string k = kv.substr(0, pos);
       std::string v = kv.substr(pos+1);

       m_all->add(k,v);  
    }
}


void BEnv::readFile(const char* dir, const char* name)
{
    LOG(verbose) << " dir " << dir << " name " << name ; 
    m_all  = MSS::load(dir, name); 
}
void BEnv::readFile(const char* path)
{
    LOG(verbose) << " path " << path ; 
    m_path = path ? strdup(path) : NULL ;  
    m_all  = MSS::load(path); 
}

void BEnv::save(const char* dir, const char* name)
{
    MSS* mss = m_selection ? m_selection : m_all ; 
    mss->save(dir, name);
}
void BEnv::save(const char* path)
{
    MSS* mss = m_selection ? m_selection : m_all ; 
    mss->save(path);
}





void BEnv::setPrefix(const char* prefix)
{
    delete m_selection ;
    if(prefix)
    {
        char delim = ',' ;
        m_prefix = strdup(prefix);
        m_selection = m_all->makeSelection(prefix, delim);
    }
    else
    {
        m_selection = NULL ; 
        m_prefix = NULL ; 
    }
}


void BEnv::dump(const char* msg)
{
    LOG(info) << msg << " prefix " << ( m_prefix ? m_prefix : "NULL" ) ; 

    MSS* mss = m_selection ? m_selection : m_all ; 

    if(mss == NULL)
    {
       LOG(error) << "BEnv::dump FAILED TO LOAD environment map  " 
                  << " selection " << m_selection 
                  << " all " << m_all
                  ;

        return ;
    } 


    SS m = mss->getMap();

    for(SS::iterator it=m.begin() ; it != m.end() ; it++)
    {
        std::string k = it->first ; 
        std::string v = it->second ; 

        std::cout << " k " << std::setw(30) << k 
                  << " v " <<std::setw(100) << v 
                  << std::endl ;  
    }
}


/**
BEnv::getNumberOfEnvvars
-------------------------------

Count envvar keys that start with "head" and end with "tail".
For example Geant4 data envvars start with "G4" and end with "DATA". 

**/

unsigned BEnv::getNumberOfEnvvars(const char* head, const char* tail, bool require_existing_dir ) const 
{
    MSS* mss = m_selection ? m_selection : m_all  ;
    assert(mss);  

    SS m = mss->getMap();

    unsigned count(0); 

    LOG(LEVEL)
        << " head " << head
        << " tail " << tail
        << " require_existing_dir " << require_existing_dir
        ;

    for(SS::iterator it=m.begin() ; it != m.end() ; it++)
    {
        std::string k = it->first ; 
        std::string v = it->second ; 
        bool match = BStr::StartsEndsWith(k.c_str(), head, tail ); 
        bool exists = require_existing_dir ? BFile::ExistsDir( v.c_str() ) : true ; 

        LOG(LEVEL)
            << " k " << std::setw(30) << k 
            << " v " <<std::setw(100) << v 
            << " match " << match 
            << " exists " << exists
            ; 

        if( match && exists ) count += 1 ; 
    }
    return count ;
}









std::string BEnv::nativePath(const char* val)
{
    std::string p = val ; 

    bool is_fspath = 
                p.find("$") == 0 || 
                p.find("/") == 0 || 
                p.find("\\") == 0 || 
                p.find(":") == 1  ;
 

    if(!is_fspath) return val ; 

    bool is_gitbashpath = p.find("/c/") == 0 ;

    std::string bpath = is_gitbashpath ? p.substr(2) : p ;

    std::string npath = BFile::FormPath(bpath.c_str());

    LOG(LEVEL)
        << " val " << val
        << " bpath " << bpath
        << " npath " << npath
        ;    


    return npath ; 
}


void BEnv::setEnvironment(bool overwrite, bool native)
{
   if(m_selection == NULL && m_path == NULL)
   {
       LOG(warning) << "BEnv::setEnvironment SAFETY RESTRICTION : MUST setPrefix non-NULL to effect a selection OR load from a file  " ; 
       return ;  
   } 

    typedef std::map<std::string, std::string> SS ; 

    MSS* mss = m_selection ? m_selection : m_all ; 
    if(mss == NULL)
    {
       LOG(error) << "BEnv::setEnviroment FAILED TO LOAD environment map  " 
                  << " selection " << m_selection 
                  << " all " << m_all
                  ;

        return ;
    } 

    SS m = mss->getMap();

    for(SS::iterator it=m.begin() ; it != m.end() ; it++)
    {
        std::string k = it->first ; 
        std::string v = it->second ; 

        std::string nv = native ? nativePath(v.c_str()) : v ; 

        //int rc = SSys::setenvvar( NULL, k.c_str(), nv.c_str(), overwrite);
        int rc = SSys::setenvvar(  k.c_str(), nv.c_str(), overwrite);
        assert(rc == 0 );

        LOG(verbose) << "BEnv::setEnvironment" 
                   << " overwrite " << overwrite 
                   << " k " << std::setw(30) << k 
                   << " v " <<std::setw(100) << v 
                   ;
    }
}



#ifdef _MSC_VER
void BEnv::dumpEnvironment(const char* msg, const char* )
{
   LOG(warning) << msg << " NOT IMPLEMENTED ON WINDOWS " ;     
}
#else
void BEnv::dumpEnvironment(const char* msg, const char* prefix)
{
    std::vector<std::string> elem ;   
    char delim = ',' ;
    BStr::split(elem, prefix, delim );
    size_t npfx = elem.size();

    // multiple selection via delimited prefix 


   LOG(info) << msg << " prefix " << ( prefix ? prefix : "NULL" ) ;
   int i = 0;
   while(environ[i]) 
   {
      std::string kv = environ[i++] ;

      bool select = false ;   

      if(npfx == 0)
      {
         select = true ;
      }
      else 
      {
         for(size_t p=0 ; p < npfx ; p++) if(kv.find(elem[p].c_str()) == 0) select = true ; 
      }

      if(select)
      {
          //std::cerr << kv << std::endl ; 
          //std::cout << kv << std::endl ; 
          LOG(info)  << kv ; 
      }

      // surprised to find that plog is writing to std::cout ... 
      // so to distinguish the crucial path written by OpticksIDPATH 
      // need to follow plog here
   }
}
#endif




