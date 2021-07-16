
#include "scuda.h"
#include "SStr.hh"
#include "SSys.hh"
#include "NPY.hpp"
#include "Opticks.hh"
#include "GGeo.hh"
#include "GBndLib.hh"
#include "QBnd.hh"
#include "QTex.hh"
#include "OPTICKS_LOG.hh"

void test_lookup(QBnd& qb)
{
    NPY<float>* lookup = qb.lookup(); 
    const char* dir = "$TMP/QBndTest" ; 
    LOG(info) << " save to " << dir  ; 
    lookup->save(dir, "dst.npy"); 
    qb.src->save(dir, "src.npy") ; 
}


void test_descBoundary(QBnd& qb)
{
    unsigned num_boundary = qb.getNumBoundary(); 
    LOG(info) 
        << " num_boundary " << num_boundary 
        << "qb.descBoundary " 
        << std::endl 
        << qb.descBoundary()
        ;
}


void test_getBoundaryLine(QBnd& qb)
{
    const char* spec = SSys::getenvvar("QCTX_SPEC", "Acrylic///LS" ); 
    unsigned idx = qb.getBoundaryIndex(spec); 
    unsigned num_boundary = qb.getNumBoundary(); 

    enum { IMAT = 3 } ;  
    unsigned line = qb.getBoundaryLine(spec, IMAT); 
    unsigned xline = idx*4 + IMAT ; 
    LOG(info)
        << " spec " << spec 
        << " idx " << idx  
        << " line " << line  
        << " xline " << xline  
        ; 

    assert( xline == line ); 

    unsigned line_max = (num_boundary-1)*4 + IMAT ; 
    unsigned linek_max = 2*line_max + 1 ;  

    LOG(info)
        << " line_max " << line_max 
        << " linek_max " << linek_max
        << " linek_max+1 " << linek_max+1
        << " qb.tex->height " << qb.tex->height
        << " qb.tex->width " << qb.tex->width
        ;

    assert( linek_max + 1 == qb.tex->height ); 
}


void test_getMaterialLine(QBnd& qb)
{
    std::vector<std::string> materials ; 
    SStr::Split( SSys::getenvvar("QCTX_MATERIALS", "Water,LS,Pyrex,Acrylic,NonExisting" ), ',', materials ); 
    LOG(info) << " materials.size " << materials.size() ; 

    for(unsigned i=0 ; i < materials.size() ; i++)
    { 
        const char* material = materials[i].c_str() ; 
        unsigned line = qb.getMaterialLine(material); 
        std::cout 
            << " material " << std::setw(50) << material
            << " line " << line
            << std::endl 
            ;
    }
}



int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    Opticks ok(argc, argv); 
    ok.configure(); 

    GGeo* gg = GGeo::Load(&ok); 
    GBndLib* blib = gg->getBndLib(); 
    blib->createDynamicBuffers();  // hmm perhaps this is done already on loading now ?

    QBnd qb(blib) ; 

    test_descBoundary(qb); 
    test_getBoundaryLine(qb); 
    test_getMaterialLine(qb); 
    test_lookup(qb); 

    return 0 ; 
}