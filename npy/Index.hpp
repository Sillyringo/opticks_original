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

#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <vector>

#include "plog/Severity.h"

#include "NSequence.hpp"


#include "NPY_API_EXPORT.hh"
#include "NPY_HEAD.hh"

class NPY_API Index : public NSequence {
   public:
        static const plog::Severity LEVEL ; 
        static int COUNT ; 

        typedef std::vector<std::string> VS ;
   public:
        Index(const char* itemtype, const char* reldir, const char* title=NULL, bool onebased=true);
        virtual ~Index(); 
   public:
        //static Index* load(const char* pfold, const char* rfold, const char* itemtype);
        static Index* load(const char* idpath, const char* itemtype, const char* reldir);
        //static std::string directory(const char* pfold, const char* rfold);
        //void save(const char* pfold, const char* rfold);
        bool exists(const char* idpath);
        void save(const char* idpath);
        std::string description();
   public:
       // debugging only
        std::string getPath(const char* idpath, const char* prefix, bool create_idpath_dir );
        void dumpPaths(const char* idpath, const char* msg="Index::dumpPaths");
   public:
        const char* getItemType();
        const char* getRelDir();
        const char* getTitle();
        bool isOneBased();     
   public:
        void setTitle(const char* title);
   public:
        int* getSelectedPtr();
        int  getSelected();
        const char* getSelectedKey();
   public:
        // fulfil NSequence, in order to use with GAttrSequence
        unsigned int getNumKeys() const ;
        const char* getKey(unsigned int i) const ;
        unsigned int getIndex(const char* key) const ;
   private:
        void loadMaps(const char* idpath);
        void crossreference();
   public:
        void add(const VS& vs);
        void add(const char* name, unsigned int source, bool sort=true);

        void sortNames(); // currently by ascending local index : ie addition order
        std::vector<std::string>& getNames();
        bool local_order(const std::string& a, const std::string& b);
   public:
        std::string getPrefixedString(const char* tail);
        void setExt(const char* ext);
        unsigned getIndexLocal(const char* name, unsigned missing=0) const ;
        unsigned getIndexSource(const char* name, unsigned missing=0);
        unsigned getIndexSourceStarting(const char* name, unsigned missing=0) ;

        unsigned int getIndexSourceTotal();
        float        getIndexSourceFraction(const char* name);

        bool         hasItem(const char* key);
        const char* getNameLocal(unsigned int local, const char* missing=NULL) const ;
        const char* getNameSource(unsigned int source, const char* missing=NULL) const ;

        unsigned int convertLocalToSource(unsigned int local, unsigned int missing=0);
        unsigned int convertSourceToLocal(unsigned int source, unsigned int missing=0);

   public:
        unsigned int getNumItems();
        void test(const char* msg="Index::test", bool verbose=true);
        void dump(const char* msg="Index::dump");

   private:
        const char*                          m_itemtype ; 
        const char*                          m_reldir ; 
        const char*                          m_title ; 
        const char*                          m_ext ; 
        int                                  m_selected ; 
        bool                                 m_onebased ; 
        unsigned int                         m_source_total ; 
        std::map<std::string, unsigned int>  m_source ; 
        std::map<std::string, unsigned int>  m_local ; 
        std::map<unsigned int, unsigned int> m_source2local ; 
        std::map<unsigned int, unsigned int> m_local2source ; 
        std::vector<std::string>             m_names ; 
   private:
        // populated by formTable
        std::vector<std::string>             m_labels ; 
        std::vector<unsigned int>            m_codes ; 
        int                                  m_count ; 

};

#include "NPY_TAIL.hh"

