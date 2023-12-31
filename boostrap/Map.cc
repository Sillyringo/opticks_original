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

#include <string>
#include <iostream>
#include <iomanip>

#include "BFile.hh"
#include "BStr.hh"
#include "BMap.hh"

#include "Map.hh"

#include "SLOG.hh"


template <typename K, typename V>
Map<K,V>::Map()
{
}

template <typename K, typename V>
std::map<K, V>& Map<K,V>::getMap()
{
    return m_map ; 
}


template <typename K, typename V>
Map<K,V>* Map<K,V>::load(const char* dir, const char* name)
{
    if(!BFile::ExistsFile(dir, name))   
    {
       LOG(debug) << "Map<K,V>::load no such path : dir " << dir << " name " << name  ;
       return NULL ;  
    }
    Map* m = new Map<K,V>() ; 
    m->loadFromCache(dir, name);
    return m ; 
}

template <typename K, typename V>
Map<K,V>* Map<K,V>::load(const char* path)
{
    LOG(verbose) << " path " << path ; 
    if(!BFile::ExistsFile(path))
    {
       LOG(verbose) << " no path " << path ;
       return NULL ;  
    }
    else
    {
       LOG(verbose) << " Map<K,V>::load path " << path ;
    }
    Map* m = new Map<K,V>() ; 
    m->loadFromCache(path);
    return m ; 
}


template <typename K, typename V>
void Map<K,V>::loadFromCache(const char* dir, const char* name )
{
    BMap<K, V>::load( &m_map, dir, name );  
}

template <typename K, typename V>
void Map<K,V>::loadFromCache(const char* path )
{
    BMap<K, V>::load( &m_map, path );  
}


template <typename K, typename V>
void Map<K,V>::add(K key, V value)
{
    m_map[key] = value ; 
}

template <typename K, typename V>
bool Map<K,V>::hasKey(K key) const 
{
    return m_map.count(key) == 1 ; 
}

template <typename K, typename V>
V Map<K,V>::get(K key, V fallback) const 
{
    return hasKey(key) ? m_map.at(key) : fallback ; 
}







template <typename K, typename V>
void Map<K,V>::save(const char* dir, const char* name)
{
    BMap<K, V>::save( &m_map, dir, name);
}

template <typename K, typename V>
void Map<K,V>::save(const char* path)
{
    BMap<K, V>::save( &m_map, path);
}







template <typename K, typename V>
void Map<K,V>::dump(const char* msg) const 
{
    LOG(info) << msg ; 
    typedef std::map<K, V> MKV ; 
    for(typename MKV::const_iterator it=m_map.begin() ; it != m_map.end() ; it++ ) 
    {
        std::cout << std::setw(5) << it->second 
                  << std::setw(30) << it->first 
                  << std::endl ; 
    }    
}


template <typename K, typename V>
Map<K,V>* Map<K,V>::makeSelection(const char* prefix, char delim)
{
    std::vector<std::string> elem ;  
    BStr::split(elem, prefix, delim );
    size_t npfx = elem.size();
    // multiple selection via delimited prefix 

    Map* m = new Map<K,V>() ; 
    typedef std::map<K, V> MKV ; 
    for(typename MKV::iterator it=m_map.begin() ; it != m_map.end() ; it++ ) 
    {
        K k = it->first ; 
        V v = it->second ; 

        bool select = false ; 
        for(size_t p=0 ; p < npfx ; p++)
        {
            std::string pfx = elem[p];
            if(k.find(pfx.c_str()) == 0) select = true ;
            // restricts keys to std::string 
        }
        if(select) m->add(k,v);
    }
    return m ; 
}


template class Map<std::string, unsigned int>;
template class Map<std::string, std::string>;
 
