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

#include <cassert>

#include "NGLM.hpp"
#include "NPY.hpp"

#include "Opticks.hh"

#include "GMaterialLib.hh"
#include "GSurfaceLib.hh"
#include "GBndLib.hh"
#include "GItemList.hh"


#include "OPTICKS_LOG.hh"

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    Opticks ok(argc, argv) ;
    ok.configure();

    GMaterialLib* mlib = GMaterialLib::load(&ok);
    GSurfaceLib*  sbas = GSurfaceLib::load(&ok);
   
    LOG_IF(fatal, !mlib) << " failed to load mlib " ; 
    if(!mlib) return 0 ; 
    
    LOG_IF(fatal, !sbas) << " failed to load sbas : basis slib  " ; 
    if(!sbas) return 0 ; 

    GBndLib*      blib = new GBndLib(&ok) ;
    GSurfaceLib*  slib = new GSurfaceLib(&ok);

    blib->setMaterialLib(mlib);
    blib->setSurfaceLib(slib);

    LOG(info) << argv[0]
              << " blib " << blib
              << " mlib " << mlib
              << " sbas " << sbas
              << " slib " << slib
              ;

    blib->dump();
    blib->dumpMaterialLineMap();
    blib->saveAllOverride("$TMP/ggeo/GBndLibInitTest");  // writing to geocache in tests not allowed

    return 0 ; 
}


