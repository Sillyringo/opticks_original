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
#include <functional>
#include <ostream>
#include <fstream>
#include "plog/Severity.h"

#include "OpticksCSG.h"
#include "NQuad.hpp"
#include "Nuv.hpp"
#include "NPY_API_EXPORT.hh"

#include <glm/fwd.hpp>

class BRng ; 

struct nbbox ; 
struct npart ; 
struct NSceneConfig ; 

class NNodeDump2 ; 
class NNodePoints ; 

class BMeta ; 
union quad ; 


// NGLMExt
struct nmat4pair ; 
struct nmat4triple ; 

#include "NNodeEnum.hpp"

/**
nnode : CSG nodes (ie constituent nodes of small boolean CSG trees) 
======================================================================

Despite the name nnode is a "mesh-level-thing", ie its needed 
only once for each shape. Every node will reference an nnode instance.

WHY does NCSG require nnode to have boundary spec char* ? 
------------------------------------------------------------

Boundary is relevant to structure-nodes (not shape-nodes at mesh level)
so it belongs at node level up in GParts. Not here in "nnode" at mesh level. 

* Suspect nnode does not need boundary any more ?
* hmm actually that was probably a convenience for tboolean- passing boundaries in from python,
  so need to keep the capability
* GParts really needs this spec, as it has a GBndLib to convert the spec 
  into a bndIdx for laying down in buffers



**/

struct NPY_API nnode 
{
    static const plog::Severity LEVEL ; 

    void find_list_nodes( std::vector<const nnode*>& list_nodes ) const ; 
    static void find_list_nodes_r( std::vector<const nnode*>& list_nodes, const nnode* n ) ; 
    void find_list_nodes( std::vector< nnode*>& list_nodes ) ; 
    static void find_list_nodes_r( std::vector<nnode*>& list_nodes, nnode* n ) ; 

    void set_p0( const quad& q0 ); 
    void set_p1( const quad& q1 ); 
    void set_bbox(const nbbox& bb); 
    void set_bbox(float min_x, float min_y, float min_z, float max_x, float max_y, float max_z ); 


    static unsigned bb_count ; 
    static nnode* copy( const nnode* a ); // retaining vtable of subclass instances 
    nnode* make_copy() const ;  // retaining vtable of subclass instances

    static nnode* deepcopy_r( const nnode* a ); 
    nnode* make_deepcopy() const ;     // probably not deep enough 


    //virtual float operator()(const glm::vec3& p) const ;
    virtual float operator()(float px, float py, float pz) const  ;
    virtual float sdf_(const glm::vec3& pos, NNodeFrameType fr) const ;

    std::function<float(float,float,float)> sdf() const ;

    static nnode* Load(const char* treedir, const NSceneConfig* config=NULL);
    static void ResizeToFit(nnode* node, const nbbox& bb, float scale, float delta ) ;

    virtual const char* csgname() const ;  
    virtual nbbox bbox() const ;
    glm::vec4 bbox_center_extent() const ; 


    void check_primitive_bb( const nbbox& bb) const  ;
    void get_composite_bbox( nbbox& bb ) const ;
    void get_primitive_bbox( nbbox& bb ) const ;

    virtual npart part() const ;
    virtual npart srcpart() const ;
    virtual unsigned maxdepth() const ;
    virtual unsigned _maxdepth(unsigned depth) const ;
    unsigned num_serialization_nodes() const ; 
    unsigned num_tree_nodes() const ; 

    static unsigned NumNodes(unsigned height); 
    static unsigned CompleteTreeHeight( unsigned num_nodes ); 


    std::string ana_desc() const ; 
    std::string ana_brief() const ; 
    static unsigned desc_indent ; 
    virtual std::string desc() const ;
    std::string tag() const ;
    std::string id() const ;

    nnode* deepclone() const ; 
    static nnode* deepclone_r(const nnode* n, unsigned depth); 

    nnode* primclone() const ; 
    static void primcopy(nnode* c, const nnode* p) ; 

    static void Init(nnode* n, int type, nnode* left=NULL, nnode* right=NULL);

    //unsigned uncoincide(unsigned verbosity);
    //bool can_uncoincide(const nnode* a, const nnode* b) const ;


    //glm::uvec4 getCompositePoints( std::vector<glm::vec3>& surf, unsigned level, int margin , unsigned pointmask, float epsilon, const glm::mat4* tr ) const ;
    //glm::uvec4 selectBySDF(std::vector<glm::vec3>& dest, const std::vector<glm::vec3>& source, unsigned pointmask, float epsilon, const glm::mat4* tr) const ;
    //void dumpPointsSDF(const std::vector<glm::vec3>& points, float epsilon ) const ;

    //void getParPoints( std::vector<glm::vec3>& parpoi, unsigned prim_idx, unsigned level, unsigned margin, NNodeFrameType frty, unsigned verbosity  ) const;
    //void getSurfacePointsAll(       std::vector<glm::vec3>& surf,        unsigned level, int margin, NNodeFrameType fr, unsigned verbosity) const ;
    //void getSurfacePoints(          std::vector<glm::vec3>& surf, int s, unsigned level, int margin, NNodeFrameType fr, unsigned verbosity) const ;


    // back-compat : to be reworked
    void dumpSurfacePointsAll(const char* msg, NNodeFrameType fr) const ;
    


    void collectParPointsSheet(unsigned prim_idx, int sheet, unsigned level, int margin, NNodeFrameType fr, unsigned verbosity) ;
    void collectParPoints(     unsigned prim_idx,            unsigned level, int margin, NNodeFrameType fr, unsigned verbosity) ;

    const std::vector<glm::vec3>& get_par_points() const ;
    const std::vector<nuv>&       get_par_coords() const ;
 

    void getCoincidentSurfacePoints(std::vector<nuv>& coincident, int s, unsigned level, int margin, const nnode* other, float epsilon, NNodeFrameType fr) const ;
    void getCoincident(             std::vector<nuv>& coincident, const nnode* other, float epsilon=1e-5f, unsigned level=1, int margin=1, NNodeFrameType fr=FRAME_LOCAL) const ;


    glm::vec3 center() const  ;      // override if needed
    glm::vec3 direction() const  ;   // override if needed

    glm::vec3 gseeddir() const ;     
    glm::vec3 gseedcenter() const ;  

    glm::vec3 par_pos_(const nuv& uv, NNodeFrameType fr) const ;
    glm::vec3 par_pos_(const nuv& uv, const nmat4triple* triple) const ;
    glm::vec3 par_pos_local(const nuv& uv) const ;  // "transform"  local node frame
    glm::vec3 par_pos_global(const nuv& uv) const ; // "gtransform" CSG tree root node frame 


    virtual glm::vec3 par_pos_model(const nuv& uv) const ;
    virtual void      par_posnrm_model(glm::vec3& pos, glm::vec3& nrm, unsigned sheet, float fu, float fv ) const ;        


    virtual unsigned  par_nsurf() const ;
    virtual int       par_euler() const ; 
    virtual unsigned  par_nvertices(unsigned nu, unsigned nv) const ;
    virtual void      nudge(unsigned s, float delta);

    static void _par_pos_endcap(glm::vec3& pos,  const nuv& uv, const float r_, const float z_ ) ; 


    void selectSheets( std::vector<unsigned>& sheets, unsigned sheetmask ) const ;
    void generateParPoints(unsigned seed, const glm::vec4& uvdom, std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, unsigned num_total, unsigned sheetmask ) const   ;
    void generateParPointsSheet(std::vector<glm::vec3>& points, std::vector<glm::vec3>& normals, BRng& ugen, BRng& vgen, unsigned sheet, unsigned num ) const ;



    // see NNodeUncoincide
    virtual void increase_z2(float dz);
    virtual void decrease_z1(float dz);
    virtual float z1() const ; 
    virtual float z2() const ; 
    virtual float r1() const ; 
    virtual float r2() const ; 


    void update_gtransforms();
    static void update_gtransforms_r(nnode* node);

    const nmat4triple* global_transform(); 

    static const nmat4triple* global_transform(nnode* n); 


    bool      has_placement() const ; 
    bool      has_placement_translation() const ; 
    glm::vec3 get_placement_translation() const ; 

    bool      has_placement_transform() const ; 
    glm::mat4 get_placement_transform() const ;     

    bool      has_root_transform() const ; 
    glm::mat4 get_root_transform() const ;     


    static void DumpTransform( const char* msg, const nmat4triple* transform ); 
    static std::string DescTransform( const nmat4triple* transform ); 


    void set_transform( const glm::mat4& t, bool update_global ); 
    void set_translation( float x, float y, float z );
    void set_placement( const nmat4triple* plc );
    void set_centering();

    void check_tree(unsigned mask) const ;
    static void check_tree_r(const nnode* node, const nnode* parent, unsigned depth, unsigned mask);


    glm::vec3 apply_gtransform(const glm::vec4& v_) const ;

    void collect_prim_centers(std::vector<glm::vec3>& centers, std::vector<glm::vec3>& dirs, int verbosity=0);
    void collect_prim_centers(std::vector<glm::vec3>& centers, std::vector<glm::vec3>& dirs,  const nnode* p  );



    virtual void pdump(const char* msg="nnode::pdump") const ; 

    virtual void dump(const char* msg=NULL) const ;
    void dump_prim() const ;
    void dump_transform() const ;
    void dump_gtransform() const ;
    void dump_planes() const ;

    void prepare(); 
    void prepareTree(); 
    void prepareList(); 
  
    // needed for NCSG::FromNode
    static void Set_parent_links_r(nnode* node, nnode* parent);

    unsigned get_num_prim() const ;
    void collect_prim(std::vector<const nnode*>& prim) const ;
    static void collect_prim_r(std::vector<const nnode*>& prim, const nnode* node) ;

    void collect_prim_for_edit(std::vector<nnode*>& prim)  ;
    static void collect_prim_for_edit_r(std::vector<nnode*>& prim, nnode* node) ;


    bool is_ellipsoid(bool verbose=false) const ;
    void reconstruct_ellipsoid( glm::vec3& axes, glm::vec2& zcut, glm::mat4& trs_unscaled ) const ; 


    const nnode* find_one( int qtyp ) const ; 
    const nnode* find_one( int qtyp1, int qtyp2 ) const ; 
    const nnode* find_one( std::vector<int>& qtyp ) const ;  // returns NULL if none or more than one are found
    void collect_nodes( std::vector<const nnode*>& nodes, std::vector<int>& qtyp ) const ;
    static void collect_nodes_r( const nnode* n, std::vector<const nnode*>& nodes, std::vector<int>& qtyp );


    void collect_monogroup( std::vector<const nnode*>& monogroup ) const ;
    void collect_progeny( std::vector<const nnode*>& progeny, int qtyp ) const ;
    void collect_ancestors( std::vector<const nnode*>& ancestors, int qtyp) const ;
    void collect_connectedtype_ancestors( std::vector<const nnode*>& ancestors) const ;

    static void collect_progeny_r( const nnode* n, std::vector<const nnode*>& progeny, int qtyp );
    static void collect_ancestors_( const nnode* n, std::vector<const nnode*>& ancestors, int qtyp);
    static void collect_connectedtype_ancestors_( const nnode* n, std::vector<const nnode*>& ancestors, int qtyp);

    static bool is_same_union(const nnode* a, const nnode* b) ; // static
    static bool is_same_monogroup(const nnode* a, const nnode* b, int op) ; // static


    //std::string get_prim_mask_string() const ;
    //std::string get_oper_mask_string() const ;

    //unsigned    get_prim_mask() const ;
    //unsigned    get_oper_mask() const ;

    std::string get_type_mask_string() const ;
    std::string get_leaf_mask_string() const ;
    std::string get_node_mask_string() const ;
    std::string get_tree_mask_string() const ;

    unsigned    get_type_mask() const ;
    unsigned    get_leaf_mask() const ;
    unsigned    get_node_mask() const ;
    unsigned    get_tree_mask() const ;

    // type composition mask for the tree with input NNodeType to select ALL,OPERATORS,PRIMITIVES 
    unsigned get_mask(int ntyp) const ;
    static void get_mask_r(const nnode* node, int ntyp, unsigned& msk);
    std::string get_mask_string(int ntyp) const ;

    // type count for the tree : eg to give the number of CSG_TORUS present in the tree
    bool has_torus() const ; 
    bool is_box3() const ; 
    bool is_box() const ; 
    unsigned get_count(int typ) const ;
    static void get_count_r(const nnode* node, int typ, unsigned& count);




 

    void set_treeidx(int idx) ; 
    void set_treeidx_r(int treeidx_); 


    int  get_treeidx() const ; 

    void set_nudgeskip(bool nudgeskip_); 
    bool is_nudgeskip() const ; 

    void set_pointskip(bool pointskip_); 
    bool is_pointskip() const ; 


    void set_treedir(const char* treedir) ; 
    void set_boundary(const char* boundary) ; 
    // boundary spec is only actually needed at structure level, 
    // suspect the boundary member is here for testing convenience (python tboolean inputs)

    bool is_znudge_capable() const ;
    bool is_zero() const ;
    bool is_lrzero() const ;  //  l-zero AND  r-zero
    bool is_rzero() const ;   // !l-zero AND  r-zero
    bool is_lzero() const ;   //  l-zero AND !r-zero

    bool is_operator() const ;
    bool is_tree() const ; 

    bool is_primitive() const ;  // left and right are nullptr
    bool is_lr_null() const ; 
    bool is_leaf() const ;       // is not compound as determined by type
    bool is_list() const ;       // has one or more sub nodes, and left and right are nullptr :  used by CSG_(DIS)CONTIGUOUS multiunions

    bool is_unbounded() const ;
    bool is_root() const ;
    bool is_bileaf() const ;



    bool has_planes() const ;
    unsigned planeIdx() const ;
    unsigned planeNum() const ;
    void setPlaneIdx(unsigned idx); 
    void setPlaneNum(unsigned num); 

    int      subNum() const ;      // returns -1 for leaf 
    int      subOffset() const ;   // returns -1 for leaf 
    void     setSubNum(unsigned sub_num) ; 
    void     setSubOffset(unsigned sub_offset) ; 
    std::string brief() const ; 
    std::string descNodes() const ; 

    
    // ---------------------------------------------------------

    unsigned     idx ; 
    int          type ;  
    nnode*       left ; 
    nnode*       right ; 
    nnode*       parent ; 
    nnode*       other ; 

    const char* label ; 
    const char* treedir ; 
    int         treeidx ;   
    bool        nudgeskip ; 
    bool        pointskip ; 
    unsigned    depth ; 
    unsigned    subdepth ; 
    const char* boundary ; 

    const nmat4triple* placement ; 
    const nmat4triple* transform ; 
    const nmat4triple* gtransform ; 
    unsigned           gtransform_idx ; 
    unsigned           itransform_idx ; 
    bool               complement ; 
    bool               external_bbox ; 
    int                verbosity ; 

    nquad param ;     // aka q0
    nquad param1 ;    // aka q1
    nquad param2 ;    // aka q2
    nquad param3 ;    // aka q3

    std::vector<glm::vec4> planes ; 
    std::vector<nnode*>    subs ; 
    std::vector<glm::vec3> par_points ; 
    std::vector<nuv>       par_coords ; 

    BMeta*        meta ;

    NNodeDump2*   _dump ;
    nbbox*        _bbox_model ; 

    const char*  g4code ; 
    const char*  g4name ; 

    typedef std::pair<std::string, double> SD ; 
    typedef std::vector<SD> VSD ; 
    VSD* g4args ; 


    static nnode* make_node(int operator_, nnode* left=NULL, nnode* right=NULL);
    static nnode* make_operator(int operator_, nnode* left=NULL, nnode* right=NULL );

    void dump_g4code() const ;
    void write_g4code(const char* path) const ;
    static void to_g4code(const nnode* root, std::ostream& out, unsigned depth );
    static void to_g4code_r(const nnode* node, std::ostream& out, unsigned depth );

};


// TODO: get these out of header

inline nnode* nnode::make_node(int operator_, nnode* left, nnode* right )
{
    nnode* n = new nnode ;    nnode::Init(n, operator_ , left, right ); return n ;
}

struct NPY_API nunion : nnode {
    float operator()(float x, float y, float z) const ;
    static nunion* make_union(nnode* left=NULL, nnode* right=NULL);
};
struct NPY_API nintersection : nnode {
    float operator()(float x, float y, float z) const ;
    static nintersection* make_intersection(nnode* left=NULL, nnode* right=NULL);
};
struct NPY_API ndifference : nnode {
    float operator()(float x, float y, float z) const ;
    static ndifference* make_difference(nnode* left=NULL, nnode* right=NULL);
};

inline nunion* nunion::make_union(nnode* left, nnode* right)
{
    nunion* n = new nunion ;         nnode::Init(n, CSG_UNION , left, right ); return n ; 
}
inline nintersection* nintersection::make_intersection(nnode* left, nnode* right)
{
    nintersection* n = new nintersection ;  nnode::Init(n, CSG_INTERSECTION , left, right ); return n ;
}
inline ndifference* ndifference::make_difference(nnode* left, nnode* right)
{
    ndifference* n = new ndifference ;    nnode::Init(n, CSG_DIFFERENCE , left, right ); return n ;
}

inline nnode* nnode::make_operator(int operator_, nnode* left, nnode* right )
{
    nnode* node = NULL ; 
    switch(operator_) 
    {
        case CSG_UNION        : node = nunion::make_union(left , right )                      ; break ;    
        case CSG_INTERSECTION : node = nintersection::make_intersection(left , right ) ; break ;    
        case CSG_DIFFERENCE   : node = ndifference::make_difference(left , right )       ; break ;    
        default               : node = NULL ; break ; 
    }
    return node ; 
}

