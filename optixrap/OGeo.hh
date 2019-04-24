#pragma once

#include <vector>

#include "OXPPNS.hh"
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_aabb_namespace.h>
#include "plog/Severity.h"

class RayTraceConfig ; 

class Opticks ; 

class OContext ; 

//class GGeo ; 
//class GGeoBase ; 
class GGeoLib ; 
class GMergedMesh ; 
class GBuffer ; 
template <typename S> class NPY ;

// used by OEngine::initGeometry

/**
OGeo
=====

Canonical OGeo instance resides in OScene and is
instanciated and has its *convert* called from OScene::init.
OScene::convert loops over the GMergedMesh within GGeo 
converting them into OptiX geometry groups. The first 
GMergedMesh is assumed to be non-instanced, the remainder
are expected to be instanced with appropriate 
transform and identity buffers.



Crucial OptiX geometrical members:


*(optix::Group)m_top*
     group holding the below

*(optix::GeometryGroup)m_geometry_group*
     single child converted from merged mesh zero, with global geometry

*(optix::Group)m_repeated_group*
     usually multiple children (order ~5) converted from merged mesh > 0, with instanced geometry 


**/

#include "OGeoStat.hh"






#include "OXRAP_API_EXPORT.hh"
class OXRAP_API  OGeo 
{
public:
    struct OGeometry 
    {
       optix::Geometry           g ; 
#if OPTIX_VERSION >= 60000
       optix::GeometryTriangles  gt ; 
#endif
    };


    static const plog::Severity LEVEL ; 
    static const char* BUILDER ; 
    static const char* TRAVERSER ; 

    OGeo(OContext* ocontext, Opticks* ok, GGeoLib* geolib, const char* builder=NULL, const char* traverser=NULL);
    void setTop(optix::Group top);
    void setVerbose(bool verbose=true);
    const char* description(const char* msg="OGeo::description");
public:
    void convert();

private:
    void init();
    void convertMergedMesh(unsigned i);
    void dumpStats(const char* msg="OGeo::dumpStats");
public:
    template <typename T> static     optix::Buffer CreateInputUserBuffer(optix::Context& ctx, NPY<T>* src, unsigned elementSize, const char* name, const char* ctxname_informational, unsigned verbosity);
public:
    template <typename T>             optix::Buffer createInputBuffer(GBuffer* buf, RTformat format, unsigned int fold, const char* name, bool reuse=false);
    template <typename T, typename S> optix::Buffer createInputBuffer(NPY<S>*  buf, RTformat format, unsigned int fold, const char* name, bool reuse=false);
    
    template <typename T>            optix::Buffer createInputUserBuffer(NPY<T>* src, unsigned elementSize, const char* name);
public:
    optix::Group   makeRepeatedGroup(GMergedMesh* mm, bool lod );

private:
    void                     setTransformMatrix(optix::Transform& xform, const float* tdata ) ;
    optix::Acceleration      makeAcceleration(const char* builder=NULL, const char* traverser=NULL);
    optix::Material          makeMaterial();

    OGeometry*               makeOGeometry(GMergedMesh* mergedmesh, unsigned lod);
    optix::GeometryInstance  makeGeometryInstance(OGeometry* geometry, optix::Material material, unsigned instance_index);
    optix::GeometryGroup     makeGeometryGroup(optix::GeometryInstance gi, optix::Acceleration accel );

    //optix::GeometryInstance  makeGeometryInstance(GMergedMesh* mergedmesh, optix::Material& material, unsigned lod);  
private:
    optix::Geometry         makeAnalyticGeometry(GMergedMesh* mergedmesh, unsigned lod);
    optix::Geometry         makeTriangulatedGeometry(GMergedMesh* mergedmesh, unsigned lod);

#if OPTIX_VERSION >= 60000
    optix::GeometryTriangles  makeRTXTrianglesGeometry(GMergedMesh* mm, unsigned lod);
#endif

private:
    void dump(const char* msg, const float* m);


private:
    // input references
    OContext*            m_ocontext ; 
    optix::Context       m_context ; 
    optix::Group         m_top ; 
    Opticks*             m_ok ; 
    int                  m_gltf ; 
    GGeoLib*             m_geolib ;  
    const char*          m_builder ; 
    const char*          m_traverser ; 
    const char*          m_description ; 
    unsigned             m_verbosity ; 
private:
    // for giving "context names" to GPU buffer uploads
    const char*          getContextName() const ;
    unsigned             m_mmidx ; 
    unsigned             m_lodidx ; 
private:
    // locals 
    optix::GeometryGroup  m_global ; 
    optix::Group          m_repeated ; 
    RayTraceConfig*       m_cfg ; 
    std::vector<OGeoStat> m_stats ; 

};

