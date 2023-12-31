#include <stdio.h>
#include "curand_kernel.h"
#include "scuda.h"
#include "qgs.h"

__global__ void _QBnd_lookup_0(cudaTextureObject_t tex, quad4* meta, quad* lookup, int num_lookup, int width, int height )
{
    int ix = blockIdx.x * blockDim.x + threadIdx.x;
    int iy = blockIdx.y * blockDim.y + threadIdx.y;
    int index = iy * width + ix ;
    if (ix >= width | iy >= height ) return;

    // Excluding hangover threads for a 2d launch based on 1d index is a bug
    // as the hangovers would overwrite into the output.
    // Must exclude based on both ix and iy. 

    int nx = meta->q0.u.x  ; 
    int ny = meta->q0.u.y  ; 

    // HMM: this is assuming normalizedCoordinates:true thats not the normal way for boundary_tex
    float x = (float(ix)+0.5f)/float(nx) ;
    float y = (float(iy)+0.5f)/float(ny) ;

    quad q ; 
    q.f = tex2D<float4>( tex, x, y );     

#ifdef DEBUG
    // debug launch config by returning coordinates 
    printf(" ix %d iy %d index %d nx %d ny %d x %10.3f y %10.3f \n", ix, iy, index, nx, ny, x, y ); 
    q.u.x = ix ; 
    q.u.y = iy ; 
    q.u.z = index ; 
    q.u.w = nx ; 
#endif
 
    lookup[index] = q ; 
}

extern "C" void QBnd_lookup_0(
    dim3 numBlocks, 
    dim3 threadsPerBlock, 
    cudaTextureObject_t tex, 
    quad4* meta, 
    quad* lookup, 
    int num_lookup, 
    int width, 
    int height  ) 
{
    _QBnd_lookup_0<<<numBlocks,threadsPerBlock>>>( tex, meta, lookup, num_lookup, width, height );
} 


