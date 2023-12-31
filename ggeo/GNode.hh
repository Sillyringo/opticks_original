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

#include <glm/fwd.hpp>
#include "GVector.hh"

struct nmat4triple ; 
struct nbbox ; 
struct gbbox ; 

template <typename T> class GMatrix ; 
class GMesh ;
class GVolume ; 

#include "GGEO_API_EXPORT.hh"
#include "GGEO_HEAD.hh"

/**
GNode
======

**boundary indices live on the node rather than the mesh**

as there are a relatively small number of meshes and many nodes
that utilize them with different transforms

normally a single boundary per-node but allow the 
possibility of compound boundary nodes, eg for combined meshes


setBoundaryIndices
setNodeIndices
setSensorIndices

    indice setters duplicate the index into an array 
    of length num_face of the associated mesh,
    allowing simple merging when flatten a tree 
    of nodes into a single structure

TODO:

* reposition some methods between GVolume/GNode for clarity 

**/

class GGEO_API GNode {
  public:
      GNode(unsigned int index, GMatrix<float>* transform, const GMesh* mesh);
      void setIndex(unsigned int index);
      void setSelected(bool selected);
      bool isSelected() const ;
      void setCSGSkip(bool csgskip);
      bool isCSGSkip() const ;
      bool isX4SkipSolid() const ; 

      virtual ~GNode();
  private:
      void init();
  public: 
      void Summary(const char* msg="GNode::Summary");
      void dump(const char* msg="GNode::dump");
  public:
      void setParent(GNode* parent);
      void addChild(GNode* child);
      void setDescription(char* desc);
      void setName(const char* name);
      const char* getName() const ;
  public:
     void     setRepeatIndex(unsigned int index);
     unsigned getRepeatIndex() const ;  
     void     setTripletIdentity(unsigned triplet_identity);
     unsigned getTripletIdentity() const ;  
  public: 
      void setBoundaryIndices(unsigned int boundary_index);
      void setSensorIndices(unsigned int sensorIndex);
  private:
      void setNodeIndices(unsigned int index); 
  public: 
      void setBoundaryIndices(unsigned int* boundary_indices);
      void setSensorIndices(unsigned int* sensor_indices);

      std::vector<unsigned int>& getDistinctBoundaryIndices();
      void updateDistinctBoundaryIndices();
  public:
      unsigned int  getIndex() const ;
      GNode*        getParent() const ; 
      GNode*        getChild(unsigned index) const ;
      GVolume*      getChildVolume(unsigned index) const ; 
      unsigned int  getNumChildren() const ;

      char*         getDescription() const ;
      gfloat3*      getLow();
      gfloat3*      getHigh();
      const GMesh*  getMesh() const ;
      unsigned      getMeshIndex() const ;
  public:
      unsigned int* getNodeIndices() const ;
      unsigned int* getBoundaryIndices() const ;
      unsigned int* getSensorIndices() const ;
  public:
      void updateBounds();
      void updateBounds(gfloat3& low, gfloat3& high );
  public:
      glm::mat4 getTransformMat4() const ;
      glm::mat4 getInverseTransformMat4() const ;
 public:
      GMatrixF*     getTransform() const ;  // global transform
      GMatrixF* getLevelTransform() const ;  // immediate "local" node transform
  public:
      GMatrixF* getRelativeTransform(const GNode* base);  // product of transforms from beneath base node
      nbbox*    getRelativeVerticesBBox(const GNode* base) ; 
      nbbox*    getVerticesBBox() const ; 
  public:
      void setLevelTransform(GMatrixF* ltransform);
  public:
      // aiming to transition to nmat4triple eventually 
      void setLocalTransform(const nmat4triple* ltriple);   
      void setGlobalTransform(const nmat4triple* gtriple);   
      const nmat4triple* getLocalTransform() const ;  
      const nmat4triple* getGlobalTransform() const ;  
  public:
      // *calculateTransform* 
      //       successfully duplicates the global transform of a node by calculating 
      //       the product of levelTransforms (ie single PV-LV transform)
      //       from ancestors + self
      // 
      //       This can be verified for all nodes within GInstancer 
      //
      //
      GMatrixF*            calculateTransform();  
  public:
      std::vector<GNode*>& getProgeny();
      std::vector<GNode*>& getRemainderProgeny();
  public:
      std::vector<GNode*>& getAncestors();
      std::string&         getProgenyDigest();
      std::string&         getLocalDigest();
  public:
      unsigned int         getPriorProgenyCount() const ;
      unsigned int         getPriorRemainderProgenyCount() const ;
  public:
      unsigned int         getProgenyNumVertices();  // includes self when m_selfdigest is true
      GNode*               findProgenyDigest(const std::string& pdig) ;
      std::vector<const GNode*>  findAllProgenyDigest(std::string& dig);
      std::vector<const GNode*>  findAllInstances(unsigned ridx, bool inside, bool honour_selection );
  private:
      std::string          meshDigest();
      std::string          localDigest();
      static std::string   localDigest(std::vector<GNode*>& nodes, GNode* extra=NULL);
  private:
      void collectProgeny(std::vector<GNode*>& progeny) ;
      void collectRemainderProgeny(std::vector<GNode*>& global_progeny) ;
  private:
      void collectAllProgenyDigest(std::vector<const GNode*>& match, std::string& dig);
      void collectAllInstances(    std::vector<const GNode*>& match, unsigned ridx, bool inside, bool honour_selection );
  private:
      bool                m_selfdigest ; // when true getProgenyDigest includes self node 
      bool                m_csgskip ; 
      bool                m_selected ;
  protected: 
      unsigned int        m_index ; 
  private:
      GNode*              m_parent ; 
      std::vector<GNode*> m_children ;
      char*               m_description ;
  private: 
      GMatrixF*           m_transform ; 
      GMatrixF*           m_ltransform ; 
  private: 
      const nmat4triple*  m_gtriple ; 
      const nmat4triple*  m_ltriple ; 
  protected: 
      const GMesh*        m_mesh ; 
  private: 
      gfloat3*            m_low ; 
      gfloat3*            m_high ; 
  private: 
      unsigned int*       m_boundary_indices ;
      unsigned int*       m_sensor_indices ;
      unsigned int*       m_node_indices ;
      const char*         m_name ; 
  private: 
      std::string         m_local_digest ; 
      std::string         m_progeny_digest ; 
      std::vector<GNode*> m_progeny ; 
      std::vector<GNode*> m_remainder_progeny ; 
      std::vector<GNode*> m_ancestors ; 
      unsigned int        m_progeny_count ; 
      unsigned int        m_remainder_progeny_count ; 
      unsigned int        m_repeat_index ; 
      unsigned int        m_triplet_identity ; 
      unsigned int        m_progeny_num_vertices ;
  private: 
      std::vector<unsigned int> m_distinct_boundary_indices ;
};

#include "GGEO_TAIL.hh"

