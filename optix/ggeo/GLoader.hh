#pragma once

#include "stdlib.h"


// npy-
class Types ; 

class GCache ; 
class GGeo ; 
class GMergedMesh ; 
class GDrawable ; 
class GBoundaryLibMetadata ; 
class GItemIndex ; 
class GColors ; 
class GBuffer ; 


class Lookup ; 

class GLoader {
     public:
         typedef GGeo* (*GLoaderImpFunctionPtr)(const char* );

    public:
         GLoader();
         void setTypes(Types* types);
         void setCache(GCache* cache);
         void setImp(GLoaderImpFunctionPtr imp);
         void load(bool nogeocache=false);

    public:
         //static const char* identityPath( const char* envprefix);
         void Summary(const char* msg);

    public:
         GGeo*                  getGGeo();
         GMergedMesh*           getMergedMesh();
         GBoundaryLibMetadata*  getMetadata();
         GDrawable*             getDrawable();

         GItemIndex*            getMaterials();
         GItemIndex*            getSurfaces();
         GItemIndex*            getFlags();
         GItemIndex*            getMeshes();

         GColors*               getColors();
         Lookup*                getMaterialLookup();
         GBuffer*               getColorBuffer();

    private:
         Types*                    m_types ; 
         GCache*                   m_cache ; 
         GLoaderImpFunctionPtr     m_imp ;  
         GGeo*                     m_ggeo ;    
         GMergedMesh*              m_mergedmesh ;
         GBoundaryLibMetadata*     m_metadata ;

         GItemIndex*               m_materials ;
         GItemIndex*               m_surfaces ;
         GItemIndex*               m_flags ;
         GItemIndex*               m_meshes ;

         GColors*                  m_colors ;  
         Lookup*                   m_lookup ; 
         GBuffer*                  m_color_buffer ; 
     
};

inline GLoader::GLoader() 
   :
   m_types(NULL),
   m_cache(NULL),
   m_ggeo(NULL),
   m_mergedmesh(NULL),
   m_metadata(NULL),
   m_materials(NULL),
   m_surfaces(NULL),
   m_flags(NULL),
   m_meshes(NULL),
   m_colors(NULL),
   m_lookup(NULL),
   m_color_buffer(NULL)
{
}




// setImp : sets implementation that does the actual loading
// using a function pointer to the implementation 
// avoids ggeo-/GLoader depending on all the implementations

inline void GLoader::setImp(GLoaderImpFunctionPtr imp)
{
    m_imp = imp ; 
}
inline void GLoader::setTypes(Types* types)
{
    m_types = types ; 
}
inline void GLoader::setCache(GCache* cache)
{
    m_cache = cache ; 
}


inline GGeo* GLoader::getGGeo()
{
    return m_ggeo ; 
}


inline GMergedMesh* GLoader::getMergedMesh()
{
    return m_mergedmesh ; 
}
inline GDrawable* GLoader::getDrawable()
{
    return (GDrawable*)m_mergedmesh ; 
}
inline GBoundaryLibMetadata* GLoader::getMetadata()
{
    return m_metadata ; 
}
inline Lookup* GLoader::getMaterialLookup()
{
    return m_lookup ; 
}

inline GColors* GLoader::getColors()
{
    return m_colors ; 
}

inline GItemIndex* GLoader::getMaterials()
{
    return m_materials ; 
}
inline GItemIndex* GLoader::getSurfaces()
{
    return m_surfaces ; 
}
inline GItemIndex* GLoader::getFlags()
{
    return m_flags ; 
}
inline GItemIndex* GLoader::getMeshes()
{
    return m_meshes ; 
}

inline GBuffer* GLoader::getColorBuffer()
{
    return m_color_buffer  ; 
}





