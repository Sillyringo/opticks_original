#pragma once
/**
SCenterExtentFrame
===================

See also 

* ana/tangential.py
* ana/pvprim.py
* ana/pvprim1.py
* ana/symtran.py
* ana/tangential.cc

**/
#include <glm/glm.hpp>
#include "SYSRAP_API_EXPORT.hh"

template<typename T>
struct SYSRAP_API SCenterExtentFrame 
{
    // convert between coordinate systems
    static void CartesianToSpherical( glm::tvec3<T>& radius_theta_phi, const glm::tvec4<T>& xyzw ); 
    static void SphericalToCartesian( glm::tvec4<T>& xyzw, const glm::tvec3<T>& radius_theta_phi );

    // rotation matrices between conventional XYZ and tangential RTP cartesian frames 
    static glm::tmat4x4<T> XYZ_to_RTP( T theta, T phi );
    static glm::tmat4x4<T> RTP_to_XYZ( T theta, T phi );

    SCenterExtentFrame( float  _cx, float  _cy, float  _cz, float  _extent, bool rtp_tangential ) ; 
    SCenterExtentFrame( double _cx, double _cy, double _cz, double _extent, bool rtp_tangential ) ; 

    void init();  
    void dump(const char* msg="SCenterExtentFrame::dump") const ; 

    glm::tvec4<T> ce ;    // center extent 
    bool          rtp_tangential ; 

    glm::tvec3<T> rtp ;   // radius_theta_phi 
    glm::tvec4<T> xyzw ;  

    glm::tmat4x4<T> scale ; 
    glm::tmat4x4<T> iscale ; 
    glm::tmat4x4<T> translate ; 
    glm::tmat4x4<T> itranslate ; 
    glm::tmat4x4<T> rotate ; 
    glm::tmat4x4<T> irotate ; 

    glm::tmat4x4<T> model2world ; 
    glm::tmat4x4<T> world2model ; 

    const T* model2world_data ; 
    const T* world2model_data ; 


}; 

