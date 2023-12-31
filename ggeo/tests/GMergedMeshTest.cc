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

//  op --mm
//  op --j1707 --mm
//  op --j1707 --mm --debugger
//
//
#include "Opticks.hh"

#include "GVector.hh"
#include "GMergedMesh.hh"
#include "OPTICKS_LOG.hh"



void test_GMergedMesh_Dump(GMergedMesh* mm)
{
    mm->Summary("mm loading");
    mm->dump("mm dump", 10);
    mm->dumpVolumes("dumpVolumes");

    unsigned int numVolumes = mm->getNumVolumes();
    unsigned int numVolumesSelected = mm->getNumVolumesSelected();

    LOG(info) 
                  << " numVolumes " << numVolumes       
                  << " numVolumesSelected " << numVolumesSelected ;      


    for(unsigned int i=0 ; i < numVolumes ; i++)
    {
        gbbox bb = mm->getBBox(i);
        bb.Summary("bb"); 
    }


    GBuffer* idbuf = mm->getIdentityBuffer();
    idbuf->dump<unsigned int>("idbuf");

    for(unsigned int i=0 ; i < mm->getNumVolumes() ; i++)
    {
        guint4 id = mm->getIdentity(i);
        LOG(info) << id.description() ; 
    }

    //mm->getLow()->Summary("low");
    //mm->getHigh()->Summary("high");
}



void test_GMergedMesh_MakeComposite(GMergedMesh* mm)
{
    std::vector<GMergedMesh*> mms ; 
    mms.push_back(mm);
    mms.push_back(mm);

    GMergedMesh* comp = GMergedMesh::MakeComposite(mms);

    comp->dumpVolumes("test_GMergedMesh_MakeComposite.dumpVolumes");
#ifdef WITH_COMPONENT
    comp->dumpComponents("test_GMergedMesh_MakeComposite.dumpComponents");
#endif

    const char* dir = "$TMP/test_GMergedMesh_MakeComposite" ; 
    comp->save(dir);    

    GMergedMesh* comp2 = GMergedMesh::Load(dir);

    comp2->dumpVolumes("test_GMergedMesh_MakeComposite.dumpVolumes.comp2");
#ifdef WITH_COMPONENT
    comp2->dumpComponents("test_GMergedMesh_MakeComposite.dumpComponents.comp2");
#endif

}


void test_GMergedMesh_MakeLODComposite(GMergedMesh* mm, unsigned levels)
{
    GMergedMesh* comp = GMergedMesh::MakeLODComposite(mm, levels);

    comp->dumpVolumes("test_GMergedMesh_MakeLODComposite.dumpVolumes");
#ifdef WITH_COMPONENT
    comp->dumpComponents("test_GMergedMesh_MakeLODComposite.dumpComponents");
#endif

}






int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    Opticks ok(argc, argv);
    ok.configure(); 

    // hmm geometry and mesh index dependant, 
    // changes to repeat finding algo GInstancer will change 
    // indices and counts

    //unsigned index = 1 ;  //  zero-faces
    //unsigned index = 2 ;  // works
    //unsigned index = 3 ;  // works
    //unsigned index = 4 ;  // works
    unsigned index = 5 ;  // works    
    //unsigned index = 6 ;    // NULL mm    


    GMergedMesh* mm = GMergedMesh::Load(&ok, index);

    if(!mm)
    {
        LOG(error) << "NULL mm" ;
        return 0 ; 
    } 

    unsigned numFaces =  mm->getNumFaces() ;
    unsigned numVolumes =  mm->getNumVolumes() ;

    LOG(info) << argv[0] 
              << " numFaces " << numFaces
              << " numVolumes " << numVolumes
               ; 


    if(numFaces == 0)
    {
        LOG(error) << "zero faces" ;
        return 0 ; 
    }


    test_GMergedMesh_Dump(mm); 

    //test_GMergedMesh_MakeComposite(mm); 
    //test_GMergedMesh_MakeLODComposite(mm,2) ; 
    //test_GMergedMesh_MakeLODComposite(mm,3) ; 


    return 0 ;
}
