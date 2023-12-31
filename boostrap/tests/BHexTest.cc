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

#include "BHex.cc"
#include "OPTICKS_LOG.hh"



int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    unsigned long long x0 = 0x0123456789abcdef ; 
    unsigned long long a0 = BHex<unsigned long long>::hex_lexical_cast("0123456789abcdef") ; 
    assert( x0 == a0 ); 

    unsigned long long x1 = 0xfedcba9876543210 ; 
    unsigned long long a1 = BHex<unsigned long long>::hex_lexical_cast("fedcba9876543210") ; 
    assert( x1 == a1 ); 


    unsigned long long x2 = 0xfedcba9876543210 ; 
    unsigned long long a2 = BHex<unsigned long long>::hex_lexical_cast("0xfedcba9876543210") ;   // << NB works with 0x prefix 
    assert( x2 == a2 ); 




    return 0 ; 
}
