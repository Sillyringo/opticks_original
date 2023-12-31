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

#include <string>
#include <vector>
#include <map>

#include <glm/fwd.hpp>

#include "OpticksCSG.h"

#include "NPY_API_EXPORT.hh"


/**

NGeoTestConfig
===============

Parses a configure string into the specifications of 
simple test geometries.   The specification is used
by both Geant4 and Opticks to create corresponding geometries.
The Geant4 usage is done via :doc:`../cfg4/CTestDetector`.

**/

class NPY_API NGeoTestConfig {
    public:
      // NODE is a generalization of the former SHAPE argument
       typedef enum { 
                      MODE, 
                      FRAME, 
                      BOUNDARY, 
                      PARAMETERS, 
                      NODE, 
                      ANALYTIC, 
                      DEBUG,
                      CONTROL,
                      PMTPATH,
                      TRANSFORM, 
                      CSGPATH,
                      OFFSETS,
                      NAME,
                      OUTERFIRST,
                      AUTOCONTAINER,
                      AUTOOBJECT,
                      AUTOEMITCONFIG,
                      AUTOSEQMAP,
                      UNRECOGNIZED } Arg_t ;

       typedef std::pair<std::string,std::string> KV ; 
       static const char* DEFAULT_CONFIG ; 
    public:
       static const char* MODE_; 
       static const char* FRAME_ ; 
       static const char* BOUNDARY_ ; 
       static const char* PARAMETERS_ ; 
       static const char* NODE_ ; 
       static const char* ANALYTIC_ ; 
       static const char* DEBUG_ ; 
       static const char* CONTROL_ ; 
       static const char* PMTPATH_ ; 
       static const char* TRANSFORM_ ; 
       static const char* CSGPATH_ ;   
       static const char* OFFSETS_ ; 
       static const char* NAME_ ; 
       static const char* OUTERFIRST_ ; 
       static const char* AUTOCONTAINER_ ; 
       static const char* AUTOOBJECT_ ; 
       static const char* AUTOEMITCONFIG_ ; 
       static const char* AUTOSEQMAP_ ; 
    public:

       NGeoTestConfig(const char* config);
       int getVerbosity();
    private:
       void init(const char* config);
       void configure(const char* config);
       Arg_t getArg(const char* k);
       void set(Arg_t arg, const char* s);
    private:
       void setMode(const char* s);
       void setFrame(const char* s);
       void setAnalytic(const char* s);
       void setOuterFirst(const char* s);
       void setDebug(const char* s);
       void setControl(const char* s);
       void setPmtPath(const char* s);
       void setCsgPath(const char* s);
       void setOffsets(const char* s);
       void setName(const char* s);
    private:
       void setAutoContainer(const char* s);
       void setAutoObject(const char* s);
       void setAutoEmitConfig(const char* s);
       void setAutoSeqMap(const char* s);
    private:
       void addNode(const char* s);
       void addBoundary(const char* s);
       void addParameters(const char* s);
       void addTransform(const char* s);
    public:
       const char* getBoundary(unsigned int i);
       glm::vec4 getParameters(unsigned int i);
       glm::mat4 getTransform(unsigned int i);
       //char      getNode(unsigned int i);  use of char codes, a workaround for dependency issue pre enum-unification ?
       int getTypeCode(unsigned int i);
       std::string getNodeString(unsigned int i); 

       bool      getAnalytic();
       bool      getOuterFirst();
       bool      isPmtInBox();
       bool      isBoxInBox();
       bool      isNCSG();

       const char* getAutoContainer() const ;
       const char* getAutoObject() const ;
       const char* getAutoEmitConfig() const ;
       const char* getAutoSeqMap() const ;

       const char* getMode();
       const char* getPmtPath();
       const char* getCSGPath();
       const char* getName();
       unsigned int getNumElements();

       std::vector<std::pair<std::string, std::string> >& getCfg();
       void dump(const char* msg="NGeoTestConfig::dump");
   private:
       unsigned getNumBoundaries();
       unsigned getNumParameters();
       unsigned getNumNodes();
       unsigned getNumTransforms();
   private:
       unsigned getOffset(unsigned idx);
   public: 
       unsigned getNumOffsets();
       bool isStartOfOptiXPrimitive(unsigned nodeIdx );
   private:
       const char*  m_config ; 
       const char*  m_mode ; 
       const char*  m_pmtpath ; 
       const char*  m_csgpath ; 
       const char*  m_name ; 

       const char*  m_autocontainer ; 
       const char*  m_autoobject  ; 
       const char*  m_autoemitconfig  ; 
       const char*  m_autoseqmap  ; 


       glm::ivec4   m_frame ;
       glm::ivec4   m_analytic ;
       glm::ivec4   m_outerfirst ;
       glm::vec4    m_debug ;
       glm::ivec4   m_control ;
       std::vector<std::string> m_nodes ; 
       std::vector<unsigned>    m_offsets ;  // identifies which nodes belong to which primitive via node offset indices 
       std::vector<std::string> m_boundaries ; 
       std::vector<glm::vec4>   m_parameters ; 
       std::vector<glm::mat4>   m_transforms ; 
       std::vector<KV> m_cfg ; 
};



