#include "CFG4_BODY.hh"
// op --cgdmldetector
// op --ctestdetector

#include <cstdio>

// g4-
#include "G4PVPlacement.hh"
#include "G4GDMLParser.hh"



// brap-
#include "BFile.hh"


// npy-
#include "NGLM.hpp"
#include "GLMFormat.hpp"
#include "NPY.hpp"
#include "NBoundingBox.hpp"

// okc-
#include "Opticks.hh"
#include "OpticksHub.hh"
#include "OpticksResource.hh"
#include "OpticksQuery.hh"

#include "GGeo.hh"


// want to eradicate
#include "GSurLib.hh"
#include "CSurLib.hh"

// replacing with
#include "CSurfaceLib.hh"


// cfg4-
#include "CBndLib.hh"
#include "CMaterialLib.hh"
#include "CTraverser.hh"
#include "CDetector.hh"
#include "CCheck.hh"

#include "PLOG.hh"



CDetector::CDetector(OpticksHub* hub, OpticksQuery* query)
  : 
  m_hub(hub),
  m_ok(m_hub->getOpticks()),
  m_dbgsurf(m_ok->isDbgSurf()),
  m_ggb(m_hub->getGGeoBase()),
  m_blib(new CBndLib(m_hub)),
  m_gsurlib(m_hub->getSurLib()),       // invokes the deferred GGeo::createSurLib  
  m_csurlib(new CSurLib(m_gsurlib)),
  m_query(query),
  m_resource(m_ok->getResource()),
  m_mlib(new CMaterialLib(m_hub)),
  m_slib(new CSurfaceLib(m_hub->getSurfaceLib())),   // << WIP 
  m_top(NULL),
  m_traverser(NULL),
  m_check(NULL),
  m_bbox(new NBoundingBox),
  m_verbosity(0),
  m_valid(true)
{
    init();
}


void CDetector::init()
{
    LOG(trace) << "CDetector::init" ;
}


bool CDetector::isValid()
{
    return m_valid ; 
}
void CDetector::setValid(bool valid)
{
    m_valid = valid ; 
}

void CDetector::setTop(G4VPhysicalVolume* top)
{
    m_top = top ; 

    traverse(m_top);

}

void CDetector::traverse(G4VPhysicalVolume* /*top*/)
{
    // invoked from CGDMLDetector::init OR CTestDetector::init via CDetector::setTop

    if(m_dbgsurf)
         LOG(info) << "[--dbgsurf] CDetector::traverse START " ;


    m_check = new CCheck(m_ok, m_top );

    
    m_traverser = new CTraverser(m_ok, m_top, m_bbox, m_query ); 
    m_traverser->Traverse();
    m_traverser->Summary("CDetector::traverse");

    if(m_dbgsurf)
         LOG(info) << "[--dbgsurf] CDetector::traverse DONE " ;
 
}




G4VPhysicalVolume* CDetector::Construct()
{
    return m_top ; 
}
G4VPhysicalVolume* CDetector::getTop()
{
    return m_top ; 
}

CMaterialLib* CDetector::getMaterialLib() const 
{
    return m_mlib ; 
}
CSurfaceLib* CDetector::getSurfaceLib() const 
{
    return m_slib ; 
}




NBoundingBox* CDetector::getBoundingBox()
{
    return m_bbox ; 
}

void CDetector::setVerbosity(unsigned int verbosity)
{
    m_verbosity = verbosity ; 
}



void CDetector::dumpLV(const char* msg)
{ 
    assert(m_traverser);
    m_traverser->dumpLV(msg);
}


const glm::vec4& CDetector::getCenterExtent()
{
    return m_bbox->getCenterExtent() ; 
}

void CDetector::saveBuffers(const char* objname, unsigned int objindex)
{
    assert(m_traverser);

    std::string cachedir = m_ok->getObjectPath(objname, objindex);

    NPY<float>* gtransforms = m_traverser->getGlobalTransforms(); 
    NPY<float>* ltransforms = m_traverser->getLocalTransforms(); 
    NPY<float>* center_extent = m_traverser->getCenterExtent(); 

    gtransforms->save(cachedir.c_str(), "gtransforms.npy");
    ltransforms->save(cachedir.c_str(), "ltransforms.npy");
    center_extent->save(cachedir.c_str(), "center_extent.npy");
}

unsigned int CDetector::getNumGlobalTransforms()
{
    assert(m_traverser);
    return m_traverser->getNumGlobalTransforms();
}
unsigned int CDetector::getNumLocalTransforms()
{
    assert(m_traverser);
    return m_traverser->getNumLocalTransforms();
}

glm::mat4 CDetector::getGlobalTransform(unsigned int index)
{
    assert(m_traverser);
    return m_traverser->getGlobalTransform(index);
}
glm::mat4 CDetector::getLocalTransform(unsigned int index)
{
    assert(m_traverser);
    return m_traverser->getLocalTransform(index);
}

NPY<float>* CDetector::getGlobalTransforms()
{
    assert(m_traverser);
    return m_traverser->getGlobalTransforms();
}

NPY<float>* CDetector::getLocalTransforms()
{
    assert(m_traverser);
    return m_traverser->getLocalTransforms();
}

const char* CDetector::getPVName(unsigned int index)
{
    assert(m_traverser);
    return m_traverser->getPVName(index);
}



const G4VPhysicalVolume* CDetector::getPV(unsigned index)
{
   return m_traverser->getPV(index); 
}
const G4VPhysicalVolume* CDetector::getPV(const char* name)
{
   return m_traverser->getPV(name); 
}



const G4LogicalVolume* CDetector::getLV(unsigned index)
{
   return m_traverser->getLV(index); 
}
const G4LogicalVolume* CDetector::getLV(const char* name)
{
   return m_traverser->getLV(name); 
}







//////// TODO get rid of m_pvm based methods, that rely on 
///////       manually setting m_pvm in CTestDetector



void CDetector::dumpLocalPV(const char* msg)
{
    LOG(info) << msg ; 

    typedef std::map<std::string, G4VPhysicalVolume*> MSV ; 

    for(MSV::const_iterator it=m_pvm.begin() ; it != m_pvm.end() ; it++)
    {
         std::string pvn = it->first ; 
         G4VPhysicalVolume* pv = it->second ;  

         std::cout << std::setw(40) << pvn 
                   << std::setw(40) << pv->GetName() 
                   << std::endl 
                   ;

    }
}

  
G4VPhysicalVolume* CDetector::getLocalPV(const char* name)
{
    return m_pvm.count(name) == 1 ? m_pvm[name] : NULL ; 
}

CDetector::~CDetector()
{
    //printf("CDetector::~CDetector\n");
    //G4GeometryManager::GetInstance()->OpenGeometry();
    //printf("CDetector::~CDetector DONE\n");
}




void CDetector::attachSurfaces()
{
    // invoked from CGeometry::init immediately after CTestDetector or GDMLDetector instanciation

    if(m_dbgsurf)
        LOG(info) << "[--dbgsurf] CDetector::attachSurfaces START closing gsurlib, creating csurlib  " ;


    m_slib->convert(this);


    m_gsurlib->close();  // close the GSurLib
    m_csurlib->convert(this);     


    if(m_dbgsurf)
        LOG(info) << "[--dbgsurf] CDetector::attachSurfaces DONE " ;

} 




void CDetector::export_dae(const char* dir, const char* name)
{
    std::string path_ = BFile::FormPath(dir, name);

    const G4String path = path_ ; 
    LOG(info) << "export to " << path_ ; 

    G4VPhysicalVolume* world_pv = getTop();
    assert( world_pv  );

#ifdef WITH_G4DAE 
    G4DAEParser* g4dae = new G4DAEParser ;

    G4bool refs = true ;
    G4bool recreatePoly = false ; 
    G4int nodeIndex = -1 ;   // so World is volume 0 

    g4dae->Write(path, world_pv, refs, recreatePoly, nodeIndex );
#else
    LOG(warning) << " export requires WITH_G4DAE " ; 
#endif
}



void CDetector::export_gdml(const char* dir, const char* name)
{
    std::string path_ = BFile::FormPath(dir, name);
 
    m_check->checkSurf();
 
    const G4String path = path_ ; 
    LOG(info) << "export to " << path_ ; 

    G4VPhysicalVolume* world_pv = getTop();
    assert( world_pv  );

    G4GDMLParser* g4gdml = new G4GDMLParser ;
    G4bool refs = true ;
    G4String schemaLocation = "" ; 

    g4gdml->Write(path, world_pv, refs, schemaLocation );

}





