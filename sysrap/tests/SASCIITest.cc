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

// TEST=SASCIITest om-t

#include <string>
#include <vector>

#include "SDice.hh"
#include "SASCII.hh"
#include "SAbbrev.hh"

#include "OPTICKS_LOG.hh"

void test_0()
{
    SASCII::DumpAllowed(); 

    const char* s = "Hello_World_0" ; 
    SASCII n(s) ; 
    assert( n.upper == 2 && n.lower == 8 && n.other == 2 && n.number == 1 ) ; 

    std::vector<std::string> ss = { 
        "Acrylic",
        "Air", 
        "Aluminium",
        "Bialkali",
        "DeadWater",
        "ESR",
        "Foam",
        "GdDopedLS",
        "IwsWater",
        "LiquidScintillator",
        "MineralOil",
        "Nitrogen",
        "NitrogenGas",
        "Nylon",
        "OwsWater",
        "PPE",
        "PVC",
        "Pyrex",
        "Rock",
        "StainlessSteel",
        "Tyvek",
        "UnstStainlessSteel",
        "Vacuum",
        "OpaqueVacuum",
        "Water",
        "GlassSchottF2"
    } ;


    for(unsigned i=0 ; i < ss.size() ; i++)
    {
        SASCII n(ss[i].c_str());  
        std::cout 
              << std::setw(30) << n.s 
              << " firstUpper(1) " << std::setw(5) << n.getFirstUpper(1) 
              << " firstUpper(2) " << std::setw(5) << n.getFirstUpper(2) 
              << " firstUpper(3) " << std::setw(5) << n.getFirstUpper(3) 
              << " firstUpper(4) " << std::setw(5) << n.getFirstUpper(4)
              << std::endl ;  
    }
}


void test_dump()
{
    SASCII n("Hello"); 
    n.dump(); 

    SDice<26> rng ; 

    for(unsigned i=0 ; i < 10 ; i++)
    {
        std::string ab = n.getTwoRandom(rng); 
        std::cout << ab << std::endl ; 
    }

}

void test_UpperIndex()
{
    const char* upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ" ; 
    for(unsigned i=0 ; i < strlen(upper) ; i++)
    {
        char c = upper[i] ; 
        int ui = SASCII::UpperIndex(c) ; 
        std::cout 
            << " i: " << std::setw(2) << i 
            << " c: " << std::setw(2) << c 
            << " ui: " << std::setw(2) << ui
            << std::endl
            ;

    }
}


void test_LowerIndex()
{
    const char* lower = "abcdefghijklmnopqrstuvwxyz" ; 
    for(unsigned i=0 ; i < strlen(lower) ; i++)
    {
        char c = lower[i] ; 
        int li = SASCII::LowerIndex(c) ; 
        std::cout 
            << " i: " << std::setw(2) << i 
            << " c: " << std::setw(2) << c 
            << " li: " << std::setw(2) << li
            << std::endl
            ;

    }
}



void test_ToUpper_ToLower()
{
    std::vector<std::string> strs = {
           "fatal", "error", "warn", "info", "debug", "verb", "none" ,
           "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "VERB", "NONE" 
         } ; 
    for(unsigned i=0 ; i < strs.size() ; i++)
    {
        const char* str = strs[i].c_str() ; 
        std::cout 
           << " str " << std::setw(20) << str 
           << " SASCII::ToUpper(str) " << std::setw(20) << SASCII::ToUpper(str) 
           << " SASCII::ToLower(str) " << std::setw(20) << SASCII::ToLower(str) 
           << std::endl 
           ; 
    }
}


int main(int argc, char** argv )
{
   /*
   test_dump(); 
   test_UpperIndex();  
   test_LowerIndex();  
   */
   test_ToUpper_ToLower(); 

   return 0 ; 
}


/*

  "ADTableStainlessSteel": "AS",
        "Acrylic": "Ac",
        "Air": "Ai",
        "Aluminium": "Al",
        "Bialkali": "Bk",
        "DeadWater": "Dw",
        "ESR": "ES",
        "Foam": "Fo",
        "GdDopedLS": "Gd",
        "IwsWater": "Iw",
        "LiquidScintillator": "LS",
        "MineralOil": "MO",
        "Nitrogen": "Ni",
        "NitrogenGas": "NG",
        "Nylon": "Ny",
        "OwsWater": "Ow",
        "PPE": "PP",
        "PVC": "PV",
        "Pyrex": "Py",
        "Rock": "Rk",
        "StainlessSteel": "SS",
        "Tyvek": "Ty",
        "UnstStainlessSteel": "US",
        "Vacuum": "Vm",
        "OpaqueVacuum": "OV",
        "Water": "Wt",
        "GlassSchottF2": "F2"


*/

