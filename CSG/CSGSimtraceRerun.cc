
#include <vector>
#include <array>
#include <cstdlib>

#include "SSys.hh"
#include "SSim.hh"
#include "SPath.hh"
#include "NP.hh"

#include "CSGFoundry.h"
#include "CSGQuery.h"
#include "CSGDraw.h"

#ifdef DEBUG_RECORD
#include "CSGRecord.h"
#endif

#include "CSGSimtraceRerun.h"
#include "PLOG.hh"


CSGSimtraceRerun::CSGSimtraceRerun()
    :
    sim(SSim::Create()),
    fd(CSGFoundry::Load()),
    SELECTION(getenv("SELECTION")),
    selection(SSys::getenvintvec("SELECTION",',')),  // when no envvar gives nullptr  
    fold(SPath::Resolve("$T_FOLD", NOOP)),
    path0(SPath::Join(fold, "simtrace.npy")),
    path1(SPath::Join(fold, selection ? "simtrace_selection.npy" : "simtrace_rerun.npy" )),
    simtrace0(NP::Load(path0)),
    simtrace1(selection ? NP::Make<float>(selection->size(),2,4,4) : NP::MakeLike(simtrace0)),
    qq0((const quad4*)simtrace0->bytes()),
    qq1((quad4*)simtrace1->bytes()),  
    q(new CSGQuery(fd)),
    d(new CSGDraw(q,'Z'))
{
    init(); 
}

void CSGSimtraceRerun::init()
{
    d->draw("CSGSimtraceRerun");
    code_count.fill(0u); 
}
 
std::string CSGSimtraceRerun::desc() const
{
    std::stringstream ss ; 
    ss << "CSGSimtraceRerun::desc" << std::endl 
       << " fd " << ( fd ? "Y" : "N" ) << std::endl 
       << " fd.geom " << (( fd && fd->geom ) ? fd->geom : "-" ) << std::endl 
       << " " << CSGQuery::Label() << std::endl 
       << " path0 " << ( path0 ? path0 : "-" ) << std::endl 
       << " path1 " << ( path1 ? path1 : "-" ) << std::endl 
       << " simtrace0 " << ( simtrace0 ? simtrace0->sstr() : "-" ) << std::endl 
       << " simtrace1 " << ( simtrace1 ? simtrace1->sstr() : "-" ) << std::endl 
       << " SELECTION " << ( SELECTION ? SELECTION : "-" ) << std::endl 
       << " selection " << ( selection ? "Y" : "N" )
       << " selection.size " << ( selection ? selection->size() : -1 ) << std::endl 
       ;

    for(unsigned i=0 ; i < code_count.size() ; i++) ss << " code_count[" << i << "] " << code_count[i] << std::endl ; 
    std::string s = ss.str(); 
    return s ; 
}

std::string CSGSimtraceRerun::Desc(const quad4& isect1, const quad4& isect0) // static
{
    bool valid_isect0 = isect0.q0.f.w > isect0.q1.f.w ;   // dist > tmin
    bool valid_isect1 = isect1.q0.f.w > isect1.q1.f.w ;   // dist > tmin

    std::stringstream ss ; 
    ss << CSGQuery::Desc( isect0, "isect0", &valid_isect0 ) << std::endl ; 
    ss << CSGQuery::Desc( isect1, "isect1", &valid_isect1 ) << std::endl ; 
    std::string s = ss.str(); 
    return s ; 
}

unsigned  CSGSimtraceRerun::intersect_again(quad4& isect1, const quad4& isect0 )
{
    bool valid_isect = q->intersect_again(isect1, isect0); 
    bool valid_isect0 = isect0.q0.f.w > isect0.q1.f.w ;   // dist > tmin
    bool valid_isect1 = isect1.q0.f.w > isect1.q1.f.w ;   // dist > tmin
    unsigned code = ( unsigned(valid_isect0) << 1 ) | unsigned(valid_isect1) ;  
    assert( code < 4 ); 
    code_count[code] += 1 ; 
    code_count[4] += 1 ; 
    assert( valid_isect == valid_isect1 );  
    return code ; 
}

void CSGSimtraceRerun::intersect_again(unsigned idx, bool dump)
{
    const quad4& isect0 = qq0[idx] ; 
    quad4&       isect1 = qq1[idx] ;
    unsigned code = intersect_again(isect1, isect0); 

    if( dump || code != 3 )
    {
        std::cout << " idx " << std::setw(7) << idx << " code " << code << std::endl ; 
        std::cout << Desc(isect1, isect0) << std::endl ; 
    }
}

/**
CSGSimtraceRerun::intersect_again_selection
-----------------------------------------------

When a selection of indices is defined, by SELECTION envvar, save 
both isect0 and isect1 into the output simtrace_selection.npy array 

**/

void CSGSimtraceRerun::intersect_again_selection(unsigned i, bool dump)
{
    unsigned idx = selection->at(i) ;
    const quad4& isect0_ = qq0[idx] ;

    quad4&       isect0 = qq1[i*2+0] ;
    quad4&       isect1 = qq1[i*2+1] ;
    isect0 = isect0_ ; 

    unsigned code = intersect_again(isect1, isect0); 

    if( dump || code != 3 )
    {
        std::cout << " i " << std::setw(3) << i << " idx " << std::setw(7) << idx << " code " << code << std::endl ; 
        std::cout << Desc(isect1, isect0) << std::endl ; 
    }
}


void CSGSimtraceRerun::intersect_again()
{
    unsigned n = selection ? selection->size() : simtrace0->shape[0] ; 
    for(unsigned i=0 ; i < n ; i++) 
    {
        if( selection == nullptr )
        {
             bool dump = i % 1000 == 0 ; 
             intersect_again(i, dump); 
        }
        else
        {
             intersect_again_selection(i,true); 
        } 
    }
}

void CSGSimtraceRerun::save() const
{
    LOG(info) << " path1 " << path1 ; 
    if(selection) simtrace1->set_meta<std::string>("SELECTION", SELECTION) ; 
    simtrace1->save(path1); 
}

void CSGSimtraceRerun::report() const 
{
    LOG(info) << "t.desc " << desc() ; 
#ifdef DEBUG_RECORD
    LOG(info) << "with : DEBUG_RECORD " ; 
    CSGRecord::Dump("CSGSimtraceRerun::report"); 

    LOG(info) << " save CSGRecord.npy to fold " << fold ; 
    CSGRecord::Save(fold); 
#else
    std::cout << "not with DEBUG_RECORD : recompile with DEBUG_RECORD for full detailed recording " << std::endl ; 
#endif

#ifdef DEBUG
    std::cout << "with : DEBUG " << std::endl ; 
#else
    std::cout << "not with : DEBUG : recompile with DEBUG for full detailed recording " << std::endl ; 
#endif

}


