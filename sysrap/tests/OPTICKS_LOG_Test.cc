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

#include "OPTICKS_LOG.hh"



void test_offset_level_logging()
{
    plog::Severity level = info ; 
    sLOG(level,4)  << " hello+4 " ; 
    sLOG(level,3)  << " hello+3 " ; 
    sLOG(level,2)  << " hello+2 " ; 
    sLOG(level,1)  << " hello+1 " ; 
    sLOG(level,0)  << " hello+0 " ; 
    sLOG(level,-1) << " hello-1 " ; 
    sLOG(level,-2) << " hello-2 " ; 
    sLOG(level,-3) << " hello-3 " ; 
    sLOG(level,-4) << " hello-4 " ; 
}


void test_SLOG_SAr_dump()
{
    // use SLOG::instance to recover commandline arguments 
    SLOG* slog = SLOG::instance ; 
    LOG(info) << " slog " << slog ; 
    assert(slog && "OPTICKS_LOG is needed to instanciate SLOG"); 
    const SAr& args = slog->args ; 
    args.dump(); 
    LOG(info) << " args.argc " << args._argc ; 
    for(int i=0 ; i < args._argc ; i++)  LOG(info) << i << ":" << args._argv[i] ; 
}



int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);
    //OPTICKS_ELOG("EmbeddedLogTest");

    //test_offset_level_logging() ;
    test_SLOG_SAr_dump();

    return 0 ; 
}
// om-;TEST=OPTICKS_LOG_Test om-t
