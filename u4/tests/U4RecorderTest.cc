#include "U4RecorderTest.h"

int main(int argc, char** argv)
{ 
    OPTICKS_LOG(argc, argv); 

    //U4Material::LoadOri();  // currently needs  "source ./IDPath_override.sh" to find _ori materials
    //U4Material::LoadBnd();   // "back" creation of G4 material properties from the Opticks bnd.npy obtained from SSim::Load 
    //U4Material::LoadBnd("$A_CFBASE/CSGFoundry/SSim"); 

    // NB: dependency on $A_CFBASE/CSGFoundry/SSim means that when changing GEOM is is necessary 
    // to run the A-side first before this B-side in order to write the $A_CFBASE/CSGFoundry/SSim
 
    U4Random rnd ;             // load precooked randoms for aligned running 
    LOG(info) << rnd.desc() ; 

    std::string desc = U4RecorderTest::Desc(); 
    LOG(info) << " desc " << desc ; 

    SEventConfig::SetStandardFullDebug(); 
    SEvt sev ;    // SEvt must be instanciated before QEvent
    sev.setReldir( desc.c_str() ); 
    const char* outdir = sev.getOutputDir(); 
    LOG(info) << "outdir [" << outdir << "]" ; 
    LOG(info) << " desc [" << desc << "]" ; 
 
    sev.random = &rnd  ;  // so can use getFlatPrior within SEvt::addTag

    sframe fr = sframe::Load_("$A_FOLD/sframe.npy");  
    sev.setFrame(fr);   // setFrame tees up Gensteps 

    // NB: dependency on A_FOLD means that when changing GEOM is is necessary 
    // to run the A-side first before this B-side in order to write the $A_FOLD/sframe.npy
    // The frame is needed for transforming input photons when using OPTICKS_INPUT_PHOTON_FRAME. 

    if(U4RecorderTest::PrimaryMode() == 'T') SEvt::AddTorchGenstep();  

    if(SSys::getenvbool("DRYRUN"))
    {
        LOG(fatal) << " DRYRUN early exit " ; 
        return 0 ; 
    }

    G4RunManager* runMgr = new G4RunManager ; 
    runMgr->SetUserInitialization((G4VUserPhysicsList*)new U4Physics); 
    U4RecorderTest t(runMgr) ;  
    runMgr->BeamOn(1); 


    rnd.saveProblemIdx(outdir); 

    sev.save(); 
    LOG(info) << "outdir [" << outdir << "]" ; 
    LOG(info) << " desc [" << desc << "]" ; 

    return 0 ; 
}
