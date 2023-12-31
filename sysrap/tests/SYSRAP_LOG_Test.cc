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
#include "SArgs.hh"

#include "OPTICKS_LOG.hh"


void check(const char* msg)
{
    LOG(verbose) << msg ; 
    LOG(verbose) << msg ; 
    LOG(info) << msg ; 
    LOG(debug) << msg ; 
    LOG(warning) << msg ; 
    LOG(error) << msg ; 
    LOG(fatal) << msg ; 
}


void test_standard_usage(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    assert(SLOG::instance);

    check("local");
    SYSRAP_LOG::Check("calling SYSRAP lib from test main");
}


int main(int /*argc_*/, char** argv_)
{
    std::vector<std::string> tt  ; 
    tt.push_back( "--trace" );
    tt.push_back( "--verbose" );
    tt.push_back( "--debug" );
    tt.push_back( "--info" );
    tt.push_back( "--warning" );
    tt.push_back( "--error" );
    tt.push_back( "--fatal" );

    tt.push_back( "--TRACE" );
    tt.push_back( "--VERBOSE" );
    tt.push_back( "--DEBUG" );
    tt.push_back( "--INFO" );
    tt.push_back( "--WARNING" );
    tt.push_back( "--ERROR" );
    tt.push_back( "--FATAL" );

    tt.push_back( "--sysrap trace" );
    tt.push_back( "--sysrap verbose" );
    tt.push_back( "--sysrap debug" );
    tt.push_back( "--sysrap info" );
    tt.push_back( "--sysrap warning" );
    tt.push_back( "--sysrap error" );
    tt.push_back( "--sysrap fatal" );

    tt.push_back( "--SYSRAP trace" );
    tt.push_back( "--SYSRAP verbose" );
    tt.push_back( "--SYSRAP debug" );
    tt.push_back( "--SYSRAP info" );
    tt.push_back( "--SYSRAP warning" );
    tt.push_back( "--SYSRAP error" );
    tt.push_back( "--SYSRAP fatal" );

    tt.push_back( "--SYSRAP TRACE" );
    tt.push_back( "--SYSRAP VERBOSE" );
    tt.push_back( "--SYSRAP DEBUG" );
    tt.push_back( "--SYSRAP INFO" );
    tt.push_back( "--SYSRAP WARNING" );
    tt.push_back( "--SYSRAP ERROR" );
    tt.push_back( "--SYSRAP FATAL" );


    const char* fallback = "info" ; 
    const char* prefix = "SYSRAP" ; 

    for(unsigned j=0 ; j < 2 ; j++)
    {

    for(unsigned i=0 ; i < tt.size() ; i++)
    {
        std::string t = tt[i]; 

        SArgs* a = new SArgs(argv_[0], t.c_str()) ;

        const char* uprefix = j == 0 ? NULL : prefix ; 

        if(SLOG::instance != NULL) SLOG::instance = NULL ; 

        SLOG* pl = new SLOG(a->argc, a->argv, fallback, uprefix );

        std::stringstream ss ; 
        ss << "SLOG(..," << fallback ; 
        if(j==1) ss << "," << uprefix ;
        ss << ")" ;

        std::string label = ss.str();

        std::cout << std::setw(50) << t 
                  << std::setw(30) << label
                  << std::setw(5) << pl->level 
                  << std::setw(10) << pl->name() 
                  << std::endl
                  ; 
    } 
    }

    return 0 ; 
}
