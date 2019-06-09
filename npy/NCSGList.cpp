#include <cstring>
#include <csignal>

#include "SSys.hh"
#include "BBnd.hh"
#include "BStr.hh"
#include "BFile.hh"
#include "BTxt.hh"

#include "NBBox.hpp"
#include "NCSG.hpp"
#include "NCSGList.hpp"
#include "NGeoTestConfig.hpp"

#include "PLOG.hh"

const char* NCSGList::FILENAME = "csg.txt" ; 


bool NCSGList::ExistsDir(const char* dir)
{
    if(!dir) return false ; 
    if(!BFile::ExistsDir(dir)) return false ; 
    return true ; 
}

NCSGList* NCSGList::Load(const char* csgpath, int verbosity, bool checkmaterial)
{
    if(!csgpath) return NULL ; 

    if(verbosity < 0 ) verbosity = SSys::getenvint("VERBOSE", 0 ) ; 


    if(!NCSGList::ExistsDir(csgpath))
    {
        LOG(warning) << "NCSGList::Load missing csgpath " << csgpath ; 
        return NULL ; 
    }

    NCSGList* ls = new NCSGList(csgpath, verbosity );
    ls->load();
    if(checkmaterial) ls->checkMaterialConsistency();
    if(ls->hasContainer())
    {
        ls->adjustContainerSize(); 
    } 

    return ls ;
} 


NCSGList* NCSGList::Create(std::vector<NCSG*>& trees, const char* csgpath, int verbosity )
{
    NCSGList* ls = new NCSGList(csgpath, verbosity );

    for(unsigned i=0 ; i < trees.size() ; i++)
    {
        NCSG* tree = trees[i] ; 
        ls->add(tree) ; 
    }
    return ls ; 
} 

void NCSGList::savesrc() const 
{
    unsigned numTrees = getNumTrees() ;
    std::cout << "NCSGList::savesrc"
              << " csgpath " << m_csgpath
              << " verbosity " << m_verbosity 
              << " numTrees " << numTrees
              << std::endl 
              ;

    for(unsigned i=0 ; i < numTrees ; i++)
    {
        std::string treedir = getTreeDir(i);
        NCSG* tree = getTree(i);
        tree->savesrc(treedir.c_str()); 
    }
    m_bndspec->write();
}


NCSGList::NCSGList(const char* csgpath, int verbosity)
    :
    m_csgpath(strdup(csgpath)),
    m_txtpath(strdup(BFile::FormPath(m_csgpath, FILENAME).c_str())),
    m_verbosity(verbosity),
    m_bndspec(new BTxt(m_txtpath)),
    m_universe(NULL),
    m_container_bbox()  
{
    init();
}

void NCSGList::init()
{
    init_bbox(m_container_bbox) ;
}

std::vector<NCSG*>& NCSGList::getTrees()
{
    return m_trees ; 
}

std::string NCSGList::getTreeDir(unsigned idx) const 
{
    return BFile::FormPath(m_csgpath, BStr::itoa(idx));  
}

void NCSGList::add(NCSG* tree)
{
    const char* boundary = tree->getBoundary() ;
    LOG(error) << " add tree, boundary: " << boundary ; 
    m_trees.push_back(tree);  
    m_bndspec->addLine( boundary ); 
}

bool NCSGList::hasContainer() const 
{
    unsigned num_tree = m_trees.size(); 
    unsigned count(0);   
    for(unsigned i=0 ; i < num_tree ; i++)
    {
        if(m_trees[i]->isContainer()) count += 1 ; 
    }  
    assert( count == 0 || count == 1 ); 
    return count == 1 ; 
}


void NCSGList::updateBoundingBox(bool exclude_container)
{
    unsigned num_tree = m_trees.size(); 
    m_bbox.set_empty(); 

    for(unsigned i=0 ; i < num_tree ; i++)
    {
        NCSG* tree = m_trees[i] ; 
        nbbox bba = tree->bbox_analytic();  

        if(!tree->isContainer() && exclude_container)
        {
            m_bbox.include(bba);
        } 
    }  
}

void NCSGList::adjustContainerSize()
{
    assert( hasContainer() ); 
    NCSG* container = findContainer(); 
    assert(container); 

    bool exclude_container = true ; 
    updateBoundingBox(exclude_container); 

    float scale = container->getContainerScale(); // hmm should be prop of the list not the tree ? 
    float delta = 0.f ; 
    container->adjustToFit(m_bbox, scale, delta );

    nbbox bba2 = container->bbox_analytic();
    m_bbox.include(bba2);   // update for the auto-container, used by NCSGList::createUniverse

    container->export_();  // after changing geometry must re-export to update the buffers destined for upload to GPU 
}





/**
NCSGList::load
------------------

**/


#ifdef OLD_LOAD
void NCSGList::load()
{
    assert(m_trees.size() == 0);
    assert(m_bndspec) ; 

    bool exists = BFile::ExistsFile(m_txtpath ); 
    if(!exists) 
    {
        LOG(fatal) << "NCSGList::load missing " << m_txtpath ; 
    }
    //assert(exists); 

    if( exists )
    {
        m_bndspec->read();
        //m_bndspec->dump("NCSGList::load");    
    }

    unsigned nbnd = m_bndspec ? m_bndspec->getNumLines() : 0 ;

    LOG(info) << "NCSGList::load"
              << " VERBOSITY " << m_verbosity 
              << " basedir " << m_csgpath 
              << " txtpath " << m_txtpath 
              << " nbnd " << nbnd 
              ;


    // order is reversed so that a tree with the "container" meta data tag at tree slot 0
    // is handled last, so container_bb will then have been adjusted to hold all the others...
    // allowing the auto-bbox setting of the container
    //
    // this order flipping feels kludgy, 
    // but because of the export of the resultant bbox it aint easy to fix
    //

    for(unsigned j=0 ; j < nbnd ; j++)
    {
        unsigned idx = nbnd - 1 - j ;     // idx 0 is handled last 
        const char* boundary = m_bndspec->getLine(idx);

        NCSG* tree = loadTree(idx);
        tree->setBoundary(boundary);  

        nbbox bba = tree->bbox_analytic();  

       // for non-container trees updates m_container_bbox, for the container trees adopts the bbox 
        if(!tree->isContainer())
        {
            m_container_bbox.include(bba);
        } 
        else if(tree->isContainer())
        {
            float scale = tree->getContainerScale(); // hmm should be prop of the list not the tree ? 
            float delta = 0.f ; 
            tree->adjustToFit(m_container_bbox, scale, delta );

            nbbox bba2 = tree->bbox_analytic();
            m_container_bbox.include(bba2);   // update for the auto-container, used by NCSGList::createUniverse

            tree->export_();  // after changing geometry must re-export to update the buffers destined for upload to GPU 

        }
  
        // tree->export_() ;  <-- former position,   
      

        LOG(debug) << "NCSGList::load [" << idx << "] " << tree->desc() ; 

        add(tree) ; 
    }

    // back into original source order with outer first eg [outer, container, sphere]  
    std::reverse( m_trees.begin(), m_trees.end() );
}

#else

void NCSGList::load()
{
    assert(m_trees.size() == 0);
    assert(m_bndspec) ; 

    bool exists = BFile::ExistsFile(m_txtpath ); 
    assert(exists); 
    m_bndspec->read();
    //m_bndspec->dump("NCSGList::load");    

    unsigned nbnd = m_bndspec->getNumLines() ;

    LOG(info) 
        << " VERBOSITY " << m_verbosity 
        << " basedir " << m_csgpath 
        << " txtpath " << m_txtpath 
        << " nbnd " << nbnd 
        ;

    for(unsigned idx=0 ; idx < nbnd ; idx++)
    {
        const char* boundary = getBoundary(idx) ;
        NCSG* tree = loadTree(idx);
        tree->setBoundary(boundary);  
        LOG(debug) << "NCSGList::load [" << idx << "] " << tree->desc() ; 
        add(tree) ; 
    }

    //std::raise(SIGINT); 

}

#endif


       
NCSG* NCSGList::getUniverse() 
{
   /*
    No longer create universe by default, 
    as with full geomrtries NCSGLoadTest and NScanTest 
    when reading /usr/local/opticks/opticksdata/export/DayaBay_VGDX_20140414-1300/extras/csg.txt
    takes exception to the content of "extras/248" not being a bnd
    */

    if(m_universe == NULL) m_universe = createUniverse(1., 1.); 
    return m_universe ; 
}

NCSG* NCSGList::createUniverse(float scale, float delta) const 
{
    //const char* bnd0 = m_bndspec->getLine(0);
    const char* bnd0 = getBoundary(0);
    const char* ubnd = BBnd::DuplicateOuterMaterial( bnd0 ); 

    LOG(info) << "NCSGList::createUniverse"
              << " bnd0 " << bnd0 
              << " ubnd " << ubnd
              << " scale " << scale
              << " delta " << delta
              ;


    // "cheat" clone (via 2nd load) of outer volume 
    // then increase size a little 
    // this is only used for the Geant4 geometry
 
    NCSG* universe = loadTree(0) ;    
    universe->setBoundary(ubnd);  

    if( universe->isContainer() )
    {
        LOG(info) << "NCSGList::createUniverse"
                  << " outer volume isContainer (ie auto scaled) "
                  << " universe will be scaled/delted a bit from there "
                  ;
    }

    universe->adjustToFit( m_container_bbox, scale, delta ); 

    /// huh : not re-exported : this means different geometry on CPU and GPU ??


    return universe ; 
}


NCSG* NCSGList::loadTree(unsigned idx) const 
{
    std::string treedir = getTreeDir(idx);

    NCSG* tree = NCSG::Load(treedir.c_str()) ; 
    tree->setIndex(idx);  

    return tree ; 
}

NCSG* NCSGList::getTree(unsigned index) const 
{
    return m_trees[index] ;
}

/*
const char* NCSGList::getBoundary(unsigned index) const 
{
    NCSG* tree = getTree(index);
    return tree ? tree->getBoundary() : NULL  ;
}
*/

const char* NCSGList::getBoundary(unsigned idx) const
{
    assert( m_bndspec ); 
    const char* boundary = m_bndspec->getLine(idx);
    return boundary ; 
}


void NCSGList::checkMaterialConsistency() const  
{
    unsigned numTree = getNumTrees();
    for( unsigned i=1 ; i < numTree ; i++)
    {
        const char* parent = getBoundary(i-1);
        const char* self = getBoundary(i);
        assert( parent && self );

        BBnd bparent(parent); 
        BBnd bself(self); 

        assert( bparent.omat );
        assert( bparent.imat && bself.omat );
        assert( bself.imat  );

        bool consistent_imat_omat = strcmp(bparent.imat, bself.omat) == 0 ; 

        if(!consistent_imat_omat)
        {
            LOG(fatal) 
                 << " BOUNDARY IMAT/OMAT INCONSISTENT " 
                 << " bparent.imat != bself.omat " 
                 << " i " << i 
                 << " numTree " << numTree
                 ;

            std::cout 
                 << std::setw(20) 
                 << " bparent " 
                 << std::setw(50) 
                 << bparent.desc() 
                 << std::setw(20)  
                 << " bparent.imat " 
                 << std::setw(20)  
                 << bparent.imat 
                 << std::endl
                 ;

            std::cout 
                 << std::setw(20) 
                 << " bself " 
                 << std::setw(50) 
                 << bself.desc() 
                 << std::setw(20)  
                 << " bself.omat " 
                 << std::setw(20)  
                 << bself.omat 
                 << std::endl
                 ;


        }

        //assert( consistent_imat_omat );
    }
}

unsigned NCSGList::getNumTrees() const 
{
    return m_trees.size();
}


int NCSGList::polygonize()
{
    unsigned numTrees = getNumTrees() ;
    assert(numTrees > 0);

    LOG(info) << "NCSGList::polygonize"
              << " csgpath " << m_csgpath
              << " verbosity " << m_verbosity 
              << " ntree " << numTrees
              ;

    int rc = 0 ; 
    for(unsigned i=0 ; i < numTrees ; i++)
    {
        NCSG* tree = getTree(i); 
        tree->setVerbosity(m_verbosity);
        tree->polygonize();
        if(tree->getTris() == NULL) rc++ ; 
    }     
    return rc ; 
}



// invoked from GGeoTest::initCreateCSG when using --testauto option
void NCSGList::autoTestSetup(NGeoTestConfig* config)
{
    const char* autocontainer  = config->getAutoContainer();
    const char* autoobject     = config->getAutoObject();
    const char* autoemitconfig = config->getAutoEmitConfig();
    const char* autoseqmap     = config->getAutoSeqMap();

  
    LOG(info) << " NCSGList::autoTestSetup"
              << " override emitconfig/boundaries and seqmap "
              ;

    std::cout  
        << " autocontainer " << autocontainer
        << std::endl 
        << " autoobject " << autoobject
        << std::endl 
        << " autoemitconfig " << autoemitconfig
        << std::endl 
        << " autoseqmap " << autoseqmap
        << std::endl 
        ;

    unsigned num_tree = getNumTrees() ;
    for(unsigned i=0 ; i < num_tree ; i++)
    {
        NCSG* tree = getTree(i) ; 
        const char* origspec = tree->getBoundary();  

        tree->setEmitConfig( autoemitconfig );
        tree->setEmit( i == 0 ? -1 : 0 );
        tree->setBoundary( i == 0 ? autocontainer : autoobject ) ;  

        const char* autospec = tree->getBoundary();  
        const char* autoemitconfig2 = tree->getEmitConfig() ; 
       
        std::cout 
             << " i " << std::setw(3) << i 
             << " origspec " << std::setw(25) << origspec
             << " autospec " << std::setw(25) << autospec
             << " autoemitconfig2 " << autoemitconfig2
             << std::endl 
             ;
    }
}


/**

NCSGList::findEmitter
~~~~~~~~~~~~~~~~~~~~~~~

Invoked by: 

GGeoTest::findEmitter 
    
OpticksHub::findEmitter 
    via GGeoTest::findEmitter

OpticksGen::OpticksGen
    via OpticksHub::findEmitter from OpticksHub::init after geometry loaded
    yielding m_csg_emit which is used by m_emitter::

        m_emitter(m_csg_emit ? new NEmitPhotonsNPY(m_csg_emit, EMITSOURCE) : NULL ),

**/

NCSG* NCSGList::findEmitter() const 
{
    unsigned numTrees = getNumTrees() ;
    NCSG* emitter = NULL ; 
    for(unsigned i=0 ; i < numTrees ; i++)
    {
        NCSG* tree = getTree(i);
        if(tree->isEmit())
        {
           assert( emitter == NULL && "not expecting more than one emitter" );
           emitter = tree ;
        }
    }
    return emitter ; 
}

NCSG* NCSGList::findContainer() const 
{
    unsigned numTrees = getNumTrees() ;
    NCSG* container = NULL ; 
    for(unsigned i=0 ; i < numTrees ; i++)
    {
        NCSG* tree = getTree(i);
        if(tree->isContainer())
        {
           assert( container == NULL && "not expecting more than one container" );
           container = tree ;
        }
    }
    return container ; 
}



void NCSGList::dumpDesc(const char* msg) const 
{
    LOG(info) << msg ; 

    unsigned numTrees = getNumTrees() ;

    std::cout << "NCSGList::dumpDesc"
              << " csgpath " << m_csgpath
              << " verbosity " << m_verbosity 
              << " numTrees " << numTrees
              << std::endl 
              ;

    for(unsigned i=0 ; i < numTrees ; i++)
    {
         NCSG* tree = getTree(i);
         std::cout 
             << " tree " << std::setw(2) << i << " "
             << tree->desc() 
             << std::endl ;
    }
}

void NCSGList::dumpMeta(const char* msg) const 
{
    LOG(info) << msg ; 

    unsigned numTrees = getNumTrees() ;

    std::cout << "NCSGList::dumpMeta"
              << " csgpath " << m_csgpath
              << " verbosity " << m_verbosity 
              << " numTrees " << numTrees
              << std::endl 
              ;

    for(unsigned i=0 ; i < numTrees ; i++)
    {
         NCSG* tree = getTree(i);
         std::cout 
             << " tree " << std::setw(2) << i << " "
             << tree->meta() 
             << std::endl ;
    }
}


void NCSGList::dump(const char* msg) const 
{
    dumpDesc(msg);
    dumpMeta(msg);
}

void NCSGList::dumpUniverse(const char* msg) 
{
    LOG(info) << msg
              << " csgpath " << m_csgpath
              ;

    NCSG* tree = getUniverse();
    std::cout 
         << " meta " 
         << std::endl 
         << tree->meta() 
         << std::endl 
         << " desc " 
         << std::endl 
         << tree->desc() 
         << std::endl 
         ;
}



