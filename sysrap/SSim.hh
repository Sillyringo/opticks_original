#pragma once
/**
SSim.hh : Manages input arrays for QUDARap/QSim : Using Single NPFold Member
==================================================================================

SSim must be instanciated with SSim::Create prior to CSGFoundry::CSGFoundry 
Currently that is done from G4CXOpticks::G4CXOpticks 

With the old X4/GGeo workflow GGeo::convertSim which is for example invoked 
during GGeo to CSGFoundry conversion within CSG_GGeo_Convert::convertSim




Currently the SSim instance is persisted within CSGFoundry/SSim 
using NPFold functionality.  

The SSim instance provides the input arrays to QSim
which does the uploading to the device. 


HMM : Transitionally : How best to switch between alternate arrays ? 
-----------------------------------------------------------------------


**/

struct NP ; 
struct NPFold ; 
struct SBnd ; 
struct stree ; 
struct scontext ; 

#include <vector>
#include <string>
#include "plog/Severity.h"
#include "SYSRAP_API_EXPORT.hh"
#include "snam.h" 

struct SYSRAP_API SSim
{
    const char* relp ; 
    scontext* sctx ; 
    NPFold*   top ; 
    NPFold*   extra ;  
    stree*    tree ;    // instanciated with SSim::SSim


    static const plog::Severity LEVEL ; 
    static const int stree_level ; 
    static constexpr const char* RELDIR = "SSim" ; 
    static constexpr const char* EXTRA = "extra" ;
 
    static SSim* INSTANCE ; 
    static SSim* Get(); 
    static scontext* GetContext(); 
    static const char* GetContextBrief(); 
    static SSim* CreateOrReuse(); 

    static void  AddExtraSubfold(const char* k, NPFold* f); 

    static int Compare( const SSim* a , const SSim* b ) ; 
    static std::string DescCompare( const SSim* a , const SSim* b ); 

    static SSim* Create(); 
    static const char* DEFAULT ; 
    static SSim* Load(); 
    static SSim* Load_(const char* dir); 
    static SSim* Load(const char* base, const char* rel=RELDIR ); 

private:
    SSim(); 
    void init(); 
public:
    stree* get_tree() const ; 
    int lookup_mtline( int mtindex ) const ; 
public:
    // top NPFold must be populated with SSim::serialize 
    // prior to these accessors working 
    std::string desc() const ; 

    const NP* get(const char* k) const ; 
    void      set(const char* k, const NP* a) ; 

    const NP* get_bnd() const ; 
    const char* getBndName(unsigned bidx) const ; 
    int getBndIndex(const char* bname) const ; 
    const SBnd* get_sbnd() const ; 
public:
    void add_extra_subfold(const char* k, NPFold* f ); 

public:
    void save(const char* base, const char* reldir=RELDIR) const ; 
    void load(const char* base, const char* reldir=RELDIR) ; 
    void serialize(); 

public:

    /**
    TODO: MOST OF THE BELOW ARE DETAILS THAT ARE 
    ONLY RELEVANT TO TEST GEOMETRIES HENCE THEY 
    SHOULD BE RELOCATED ELSEWHERE, AND THE API 
    UP HERE SLIMMED DOWN DRASTICALLY 
    **/

    template<typename ... Args> void addFake( Args ... args ); 
    void addFake_( const std::vector<std::string>& specs ); 

    static void Add( 
        NP** opticalplus, 
        NP** bndplus, 
        const NP* optical, 
        const NP* bnd,  
        const std::vector<std::string>& specs 
        ); 

    static NP*  AddOptical( 
        const NP* optical, 
        const std::vector<std::string>& bnames, 
        const std::vector<std::string>& specs 
        ) ; 

    static NP*  AddBoundary( 
        const NP* src, 
        const std::vector<std::string>& specs 
        ); 

    static void GetPerfectValues( 
        std::vector<float>& values, 
        unsigned nk, unsigned nl, unsigned nm, const char* name 
        ); 

    bool hasOptical() const ; 
    std::string descOptical() const ; 
    static std::string DescOptical(const NP* optical, const NP* bnd ); 

    static std::string GetItemDigest( const NP* bnd, int i, int j, int w ); 
    bool   findName( int& i, int& j, const char* qname ) const ; 


};


