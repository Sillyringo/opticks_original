#include "scuda.h"
#include "sqat4.h"

#include <optix_device.h>

#define DEBUG6 1
#include "csg_intersect_leaf.h"
#include "csg_intersect_node.h"
#include "csg_intersect_tree.h"

#include "CSGPrim.h"
#include "CSGNode.h"

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

rtDeclareVariable(float3, position,         attribute position, );  
rtDeclareVariable(float3, shading_normal,   attribute shading_normal, );  
rtDeclareVariable(unsigned,  intersect_identity,   attribute intersect_identity, );  

rtDeclareVariable(unsigned, identity,  ,);   
// "identity" is planted into pergi["identity"] 


// per solid context, not global 
rtBuffer<CSGPrim> prim_buffer;

// global context : shim optix::Buffers facade for CUDA buffers setup in Six::createContextBuffers
rtBuffer<CSGNode> node_buffer;
rtBuffer<qat4>    itra_buffer;
rtBuffer<float4>  plan_buffer;

/**
As the primIdx argument is in 0:num_prim-1 need separate prim_buffer per geometry 
unlike nodes and itra where context level node_buffer and itra_buffer allows 
the pre-7 machinery to more closely match optix7
**/

RT_PROGRAM void intersect(int primIdx)
{
    const CSGPrim* prim = &prim_buffer[primIdx] ;   
    int nodeOffset = prim->nodeOffset() ;  
    const CSGNode* node = &node_buffer[nodeOffset] ; 
    const float4* plan = &plan_buffer[0] ;  
    const qat4*   itra = &itra_buffer[0] ;  

    float4 isect ; 
    bool valid_isect = intersect_prim(isect, node, plan, itra, ray.tmin , ray.origin, ray.direction ) ; 

#ifdef DEBUG_SIX
    rtPrintf("//SIX/geo_OptiXTest.cu:intersect identity %d primIdx %d nodeOffset %d valid_isect %d isect.w %10.4f \n", 
         identity, primIdx, nodeOffset, valid_isect, isect.w ); 
#endif

    if(valid_isect)
    {
        if(rtPotentialIntersection(isect.w))
        {
            position = ray.origin + isect.w*ray.direction ;   
            shading_normal = make_float3( isect.x, isect.y, isect.z ); 
            intersect_identity = (( (1u+primIdx) & 0xff ) << 24 ) | ( identity & 0x00ffffff ) ; 
            rtReportIntersection(0);
        }
    }
}

RT_PROGRAM void bounds (int primIdx, float result[6])
{
    const CSGPrim* prim = &prim_buffer[primIdx] ; 
    int nodeOffset = prim->nodeOffset() ;  
    const float* aabb = prim->AABB();  

    result[0] = *(aabb+0); 
    result[1] = *(aabb+1); 
    result[2] = *(aabb+2); 
    result[3] = *(aabb+3); 
    result[4] = *(aabb+4); 
    result[5] = *(aabb+5); 

#ifdef DEBUG_SIX
    rtPrintf("//SIX/geo_OptiXTest.cu:bounds identity %d primIdx %d nodeOffset %d aabb %10.3f %10.3f %10.3f   %10.3f %10.3f %10.3f  \n", 
         identity, primIdx, nodeOffset, 
         result[0], result[1], result[2],  
         result[3], result[4], result[5] 
        ); 
#endif
}


