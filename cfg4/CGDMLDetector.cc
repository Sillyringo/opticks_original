// cfg4--;op --cgdmldetector

#include "CFG4_BODY.hh"

#include "BFile.hh"
#include "GLMFormat.hpp"

// ggeo-
#include "GMaterial.hh"
#include "GSurfaceLib.hh"

// okc-
#include "Opticks.hh"
#include "OpticksResource.hh"

// okg-
#include "OpticksHub.hh"   

#include "CMaterialLib.hh"
#include "CTraverser.hh"    // m_traverser resides in base 
#include "CGDMLDetector.hh"

#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4GDMLParser.hh"


#include "PLOG.hh"


CGDMLDetector::CGDMLDetector(OpticksHub* hub, OpticksQuery* query)
    : 
    CDetector(hub, query),
    m_level(info)
{
    LOG(m_level) << "." ; 
    init();
}

CGDMLDetector::~CGDMLDetector()
{
}


void CGDMLDetector::init()
{
    const char* path = m_ok->getGDMLPath();
    bool exists = BFile::ExistsFile(path);
    if( !exists )
    {
         LOG(error)
              << "CGDMLDetector::init" 
              << " PATH DOES NOT EXIST "
              << " path " << path
              ; 

         setValid(false);  
         return ; 
    }

    LOG(m_level) << "parse " << path ; 
    G4VPhysicalVolume* world = parseGDML(path);

    setTop(world);   // invokes *CDetector::traverse*

    addMPT();
    attachSurfaces();

}

G4VPhysicalVolume* CGDMLDetector::parseGDML(const char* path) const 
{
    bool validate = false ; 
    bool trimPtr = false ; 
    G4GDMLParser parser;
    parser.SetStripFlag(trimPtr);
    parser.Read(path, validate);
    return parser.GetWorldVolume() ;
}


void CGDMLDetector::saveBuffers()
{
    // split off to allow setting idpath override whilst testing
    CDetector::saveBuffers("CGDMLDetector", 0);    
}


void CGDMLDetector::addMPT()
{
    // GDML exported by geant4 that comes with nuwa lack material properties 
    // so use the properties from the G4DAE export 

    unsigned int nmat = m_traverser->getNumMaterials();
    unsigned int nmat_without_mpt = m_traverser->getNumMaterialsWithoutMPT();

    if(nmat > 0 && nmat_without_mpt == nmat )
    {
        LOG(warning) << "CGDMLDetector::addMPT" 
                     << " ALL G4 MATERIALS LACK MPT "
                     << " FIXING USING G4DAE MATERIALS " 
                     ;
    } 
    else
    {
        LOG(fatal) << "CGDMLDetector::addMPT UNEXPECTED"
                   << " nmat " << nmat 
                   << " nmat_without_mpt " << nmat_without_mpt
                   ;
        assert(0);
    }
 

    unsigned int ng4mat = m_traverser->getNumMaterialsWithoutMPT() ;
    for(unsigned int i=0 ; i < ng4mat ; i++)
    {
        G4Material* g4mat = m_traverser->getMaterialWithoutMPT(i) ;
        const char* name = g4mat->GetName() ;

        const std::string base = BFile::Name(name);
        const char* shortname = base.c_str();

/*
        std::vector<std::string> elem;
        boost::split(elem,name,boost::is_any_of("/"));
        assert(elem.size() == 4 && "expecting material names like /dd/Materials/GdDopedLS " );
        const char* shortname = elem[3].c_str();
*/
        const GMaterial* ggmat = m_mlib->getMaterial(shortname);          
        assert(ggmat && strcmp(ggmat->getShortName(), shortname)==0 && "failed to find corresponding G4DAE material") ;

        LOG(debug) << "CGDMLDetector::addMPT" 
                  << " g4mat " << std::setw(45) << name
                  << " shortname " << std::setw(25) << shortname
                   ;

        G4MaterialPropertiesTable* mpt = m_mlib->makeMaterialPropertiesTable(ggmat);
        g4mat->SetMaterialPropertiesTable(mpt);
        //m_mlib->dumpMaterial(g4mat, "CGDMLDetector::addMPT");        
         
    }

    LOG(info) << "CGDMLDetector::addMPT added MPT to " <<  ng4mat << " g4 materials " ; 

}



 
