#pragma once

#ifdef __CUDACC__
/**
OSensorLib_angular_efficiency
-------------------------------

Interpolated lookup of sensor angular efficiency for local angle fractions and sensor category.

category
    0-based sensor category index, typically for a small number (<10) of sensor types
    Category -1 corresponds to the case when no angular efficiency is available for 
    a sensor resulting in a returned angular_efficiency of 1.f 
  
phi_fraction
    fraction of the 360 degree azimuthal phi angle, range 0. to 1. 
    (when there is no phi-dependence of efficiency use 0. for clarity)

theta_fraction
    fraction of the 180 degree polar theta angle, range 0. to 1. 

The sensor efficiency textures for each category are constructed by OSensorLib::convert
from the array held by okg/SensorLib 

See tests/OSensorLibTest 
**/

rtBuffer<float4,1>  OSensorLib_sensor_data ;
rtBuffer<int4,1>    OSensorLib_texid ;

static __device__ __inline__ int OSensorLib_category(unsigned sensorIndex ) 
{
    int category =  __float_as_int(OSensorLib_sensor_data[sensorIndex-1].z) ;  // 1-based sensorIndex
    return category ; 
}

static __device__ __inline__ float OSensorLib_angular_efficiency(int category, float phi_fraction, float theta_fraction  )
{
    int tex_id = category > -1 ? OSensorLib_texid[category].x : -1 ; 
    float angular_efficiency = tex_id > -1 ? rtTex2D<float>( tex_id, phi_fraction, theta_fraction ) : 1.f ;  
    return angular_efficiency ; 
}

static __device__ __inline__ float OSensorLib_simple_efficiency(unsigned sensorIndex)
{
    unsigned sensor_data_size = OSensorLib_sensor_data.size(); 
    if( sensor_data_size == 0 ) return 1.f ; 

    const float4& sensor_data = OSensorLib_sensor_data[sensorIndex-1] ;  // 1-based sensorIndex

    float efficiency_1 = sensor_data.x ; 
    float efficiency_2 = sensor_data.y ; 

    float simple_efficiency = efficiency_1*efficiency_2 ;
    return simple_efficiency ;
}

static __device__ __inline__ float OSensorLib_combined_efficiency(unsigned sensorIndex, float phi_fraction, float theta_fraction  )
{
    // not expecting sensorIndex 0 which means that the volume is not a sensor

    unsigned sensor_data_size = OSensorLib_sensor_data.size(); 
    if( sensor_data_size == 0 ) return 1.f ; 

    const float4& sensor_data = OSensorLib_sensor_data[sensorIndex-1] ;  // 1-based sensorIndex

    float efficiency_1 = sensor_data.x ; 
    float efficiency_2 = sensor_data.y ; 
    int category = __float_as_int(sensor_data.z); 
    //int identifier = __float_as_int(sensor_data.w); 

    int tex_id = category > -1 ? OSensorLib_texid[category].x : -1 ; 
    float angular_efficiency = tex_id > -1 ? rtTex2D<float>( tex_id, phi_fraction, theta_fraction ) : 1.f ;  

    float combined_efficiency = efficiency_1*efficiency_2*angular_efficiency ;
    return combined_efficiency ;
} 

#else

#include "OXPPNS.hh"

class SensorLib ; 
class OCtx ; 

template <typename T> class NPY ;

#include "plog/Severity.h"
#include "OXRAP_API_EXPORT.hh"

/**
OSensorLib
===========

Canonical m_osensorlib defaults to NULL in OScene ctor.
It is instancianted and converted by OScene::uploadSensorLib




**/

class OXRAP_API OSensorLib 
{
    public:
        static const plog::Severity LEVEL ; 
        static const char*  TEXID ; 
        static const char*  SENSOR_DATA ; 
    public:
        OSensorLib(const OCtx* octx, const SensorLib* sensorlib);
        const OCtx* getOCtx() const ;
    private:
        void init();
    public:
        const NPY<float>*  getSensorAngularEfficiencyArray() const ;
        unsigned getNumSensorCategories() const ;
        unsigned getNumTheta() const ;
        unsigned getNumPhi() const ;
        unsigned getNumElem() const ;
    public:
        int      getTexId(unsigned icat) const ;
    public:
        void convert();
    private:    
        void makeSensorDataBuffer();
        void makeSensorAngularEfficiencyTexture();
    private:    
        const OCtx*        m_octx ; 
        const SensorLib*   m_sensorlib ; 
        const NPY<float>*  m_sensor_data ; 
        const NPY<float>*  m_angular_efficiency ; 

        unsigned           m_num_dim ; 
        unsigned           m_num_cat ;
        unsigned           m_num_theta ;
        unsigned           m_num_phi ;
        unsigned           m_num_elem ;
        NPY<int>*          m_texid ; 

};

#endif
