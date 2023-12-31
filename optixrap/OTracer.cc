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

#include <iomanip>
#include <optixu/optixu.h>
#include <optixu/optixu_math_stream_namespace.h>

// brap-
//#include "STimes.hh"
#include "BTimes.hh"
#include "BTimeStamp.hh"

// npy-
#include "NGLM.hpp"
#include "GLMPrint.hpp"
#include "GLMFormat.hpp"

// okc-
#include "Opticks.hh"
#include "OpticksEntry.hh"
#include "Composition.hh"

#include "OTracer.hh"
#include "OContext.hh"


#include "PLOG.hh"
using namespace optix ; 


OTracer::OTracer(OContext* ocontext, Composition* composition) 
    :
    m_ocontext(ocontext),
    m_ok(ocontext->getOpticks()),
    m_composition(composition),
    m_resolution_scale(1),
    m_trace_times(NULL),
    m_trace_count(0),
    m_trace_prep(0),
    m_trace_time(0),
    m_entry_index(-1)
{
    init();
}


void OTracer::setResolutionScale(unsigned int resolution_scale)
{
    m_resolution_scale = resolution_scale ; 
}
unsigned OTracer::getResolutionScale() const
{
    return m_resolution_scale ; 
}
unsigned OTracer::getTraceCount() const
{
    return m_trace_count ; 
}
BTimes* OTracer::getTraceTimes() const
{
    return m_trace_times ; 
}


void OTracer::init()
{
    m_context = m_ocontext->getContext();

    // OContext::e_pinhole_camera_entry
    bool defer = true ; 

    char code = 'P' ; 
    OpticksEntry* entry =  m_ocontext->addEntry(code, "OTracer::init");
    m_entry_index = entry->getIndex();

    m_ocontext->setMissProgram(           OContext::e_radiance_ray , "constantbg.cu", "miss", defer );

    m_context[ "scene_epsilon"]->setFloat(m_composition->getNear());

    float pixeltimescale_cfg = m_ok->getPixelTimeScale(); 
    LOG(debug) << " pixeltimescale_cfg " << pixeltimescale_cfg ;
    m_context[ "pixeltimescale_cfg"]->setFloat(pixeltimescale_cfg);


    m_context[ "radiance_ray_type"   ]->setUint( OContext::e_radiance_ray );
    m_context[ "touch_ray_type"      ]->setUint( OContext::e_touch_ray );
    m_context[ "propagate_ray_type"  ]->setUint( OContext::e_propagate_ray );

    m_context[ "bg_color" ]->setFloat(  0.34f, 0.55f, 0.85f, 1.0f ); // map(int,np.array([0.34,0.55,0.85])*255) -> [86, 140, 216]
    m_context[ "bad_color" ]->setFloat( 1.0f, 0.0f, 0.0f, 1.0f );

    m_trace_times = new BTimes("OTracerTimes") ; 
}



double OTracer::trace_()
{
    LOG(debug) << "OTracer::trace_ " << m_trace_count ; 

    double t0 = BTimeStamp::RealTime();  // THERE IS A HIGHER LEVEL WAY TO DO THIS

    glm::vec3 eye ;
    glm::vec3 U ;
    glm::vec3 V ;
    glm::vec3 W ;
    glm::vec4 ZProj ;

    m_composition->getEyeUVW(eye, U, V, W, ZProj); // must setModelToWorld in composition first

    unsigned cameratype = m_composition->getCameraType();  // 0:PERSP, 1:ORTHO, 2:EQUIRECT
    unsigned pixeltime_style = m_composition->getPixelTimeStyle() ; 
    float    pixeltime_scale = m_composition->getPixelTimeScale() ; 
    float      scene_epsilon = m_composition->getNear();

    const glm::vec3 front = glm::normalize(W); 

    m_context[ "cameratype"]->setUint( cameratype ); 
    m_context[ "pixeltime_style"]->setUint( pixeltime_style ); 
    m_context[ "pixeltime_scale"]->setFloat( pixeltime_scale ); 
    m_context[ "scene_epsilon"]->setFloat(scene_epsilon); 
    m_context[ "eye"]->setFloat( make_float3( eye.x, eye.y, eye.z ) );
    m_context[ "U"  ]->setFloat( make_float3( U.x, U.y, U.z ) );
    m_context[ "V"  ]->setFloat( make_float3( V.x, V.y, V.z ) );
    m_context[ "W"  ]->setFloat( make_float3( W.x, W.y, W.z ) );
    m_context[ "front"  ]->setFloat( make_float3( front.x, front.y, front.z ) );
    m_context[ "ZProj"  ]->setFloat( make_float4( ZProj.x, ZProj.y, ZProj.z, ZProj.w ) );

    Buffer buffer = m_context["output_buffer"]->getBuffer();
    RTsize buffer_width, buffer_height;
    buffer->getSize( buffer_width, buffer_height );

    // resolution_scale 
    //
    //   1: full resolution, launch index for every pixel 
    //   2: half resolution, each launch index result duplicated into 2*2=4 pixels
    //            
    unsigned int width  = static_cast<unsigned int>(buffer_width)/m_resolution_scale ;
    unsigned int height = static_cast<unsigned int>(buffer_height)/m_resolution_scale ;
    m_context["resolution_scale"]->setUint( m_resolution_scale ) ;  

    if(m_trace_count % 100 == 0) 
         LOG(info) 
                   << " entry_index " << m_entry_index 
                   << " trace_count " << m_trace_count 
                   << " resolution_scale " << m_resolution_scale 
                   << " pixeltime_scale " << pixeltime_scale 
                   << " size(" <<  width << "," <<  height << ")"
                   << " ZProj.zw (" <<  ZProj.z << "," <<  ZProj.w << ")"
                   << " front " <<  gformat(front) 
                   ;

    double t1 = BTimeStamp::RealTime();

    unsigned int lmode = m_trace_count == 0 ? OContext::VALIDATE|OContext::COMPILE|OContext::PRELAUNCH|OContext::LAUNCH : OContext::LAUNCH ;

    //OContext::e_pinhole_camera_entry
    double dt = m_ocontext->launch( lmode,  m_entry_index,  width, height, m_trace_times );

    double t2 = BTimeStamp::RealTime();

    float trace_time = t2 - t1 ;

    m_trace_count += 1 ; 
    m_trace_prep += t1 - t0 ; 
    m_trace_time += trace_time ; 

    //LOG(info) << m_trace_times->description("OTracer::trace m_trace_times") ;

    return dt ;  
}


void OTracer::report(const char* msg)
{
    LOG(info)<< msg ; 
    if(m_trace_count == 0 ) return ; 

    LOG(info) 
       << std::endl 
       << " trace_count     " << std::setw(10) << m_trace_count  
       << " trace_prep      " << std::setw(10) << m_trace_prep   << " avg " << std::setw(10) << m_trace_prep/m_trace_count  << std::endl
       << " trace_time      " << std::setw(10) << m_trace_time   << " avg " << std::setw(10) << m_trace_time/m_trace_count  << std::endl
       ;

    m_trace_times->addAverage("launch"); 
    LOG(info) << m_trace_times->desc("OTracer::report"); 

    if(!m_ok->isProduction())
    {
        const char* runresultsdir = m_ocontext->getRunResultsDir(); 
        LOG(info) << "save to " << runresultsdir ; 
        m_trace_times->save(runresultsdir);
    }

}


