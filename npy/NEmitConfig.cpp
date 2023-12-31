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

#include <sstream>

#include "BConfig.hh"
#include "SLOG.hh"
#include "NEmitConfig.hpp"


//const char* NEmitConfig::DEFAULT = "photons=100000,wavelength=480,time=0.1,weight=1.0,posdelta=0.0,sheetmask=0" ; 
const char* NEmitConfig::DEFAULT = "photons:100000,wavelength:480,time:0.1,weight:1.0,posdelta:0.0,sheetmask:0x3f,umin:0,umax:1,vmin:0,vmax:1,diffuse:0,ctmindiffuse:0.0,ctmaxdiffuse:1.0" ; 

NEmitConfig::NEmitConfig(const char* cfg)  
    :
    bconfig(new BConfig(cfg ? cfg : DEFAULT,',',":")),
    verbosity(0),
    photons(100),
    wavelength(400),
    time(0.01f),
    weight(1.0f),
    posdelta(0.f),
    sheetmask("0xffff"),
    umin(0.f),
    umax(1.f),
    vmin(0.f),
    vmax(1.f),
    diffuse(0),
    ctmindiffuse(0.),
    ctmaxdiffuse(1.)
{
    LOG(debug) << "NEmitConfig::NEmitConfig"
              << " cfg [" << ( cfg ? cfg : "NULL" ) << "]"
              ;

    bconfig->addInt("verbosity", &verbosity );
    bconfig->addInt("photons", &photons );
    bconfig->addInt("wavelength", &wavelength );
    bconfig->addFloat("time", &time );
    bconfig->addFloat("weight", &weight );
    bconfig->addFloat("posdelta", &posdelta );
    bconfig->addString("sheetmask", &sheetmask );
    bconfig->addFloat("umin", &umin );
    bconfig->addFloat("umax", &umax );
    bconfig->addFloat("vmin", &vmin );
    bconfig->addFloat("vmax", &vmax );
    bconfig->addInt("diffuse", &diffuse );
    bconfig->addFloat("ctmindiffuse", &ctmindiffuse );
    bconfig->addFloat("ctmaxdiffuse", &ctmaxdiffuse );

    bconfig->parse();


    assert( umin >= 0 && umin <= 1.) ;
    assert( umax >= 0 && umax <= 1.) ;
    assert( vmin >= 0 && umin <= 1.) ;
    assert( vmax >= 0 && vmax <= 1.) ;

    assert( umax >= umin );
    assert( vmax >= vmin );

}


std::string NEmitConfig::desc() const 
{
    std::stringstream ss ;
    ss << bconfig->desc() ; 
    return ss.str();
}


void NEmitConfig::dump(const char* msg) const
{
    LOG(info) << bconfig->cfg ; 
    bconfig->dump(msg);
}


