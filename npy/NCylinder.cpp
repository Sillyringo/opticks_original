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


#include <iostream>
#include <cmath>
#include <cassert>
#include <cstring>

#include "NGLMExt.hpp"
#include "nmat4triple.hpp"

// sysrap-
#include "OpticksCSG.h"

// npy-
#include "NCylinder.hpp"
#include "NBBox.hpp"
#include "NPlane.hpp"
#include "NPart.hpp"
#include "Nuv.hpp"
#include "NCone.hpp"

#include "SLOG.hh"

#include "NCylinder.h"

const plog::Severity ncylinder::LEVEL = SLOG::EnvLevel("ncylinder", "DEBUG") ; 

ncylinder* ncylinder::Create(const nquad& param, const nquad& param1, bool old )  // static
{
    ncylinder* n = new ncylinder ; 
    nnode::Init(n, old ? CSG_OLDCYLINDER : CSG_CYLINDER) ; 

    n->param = param ; 
    n->param1 = param1 ;

    bool z_ascending = n->z2() > n->z1() ;
    if(!z_ascending) n->pdump("init_cylinder z_ascending FAIL ");
    assert( z_ascending  );

    return n ; 
}


ncylinder* ncylinder::Create(float radius_, float z1_, float z2_, bool old )  // static 
{
    nquad param, param1 ;

    param.f = {0,0,0,radius_} ;

    param1.f.x = z1_ ; 
    param1.f.y = z2_ ; 
    param1.u.z = 0u ; 
    param1.u.w = 0u ; 

    return Create(param, param1, old  ); 
}

ncylinder* ncylinder::Create() // static 
{
    return Create(10.f, -5.f, 15.f, false ); 
}


ncylinder* ncylinder::Create(float x0, float y0, float z0, float w0, float x1, float y1, float z1, float w1) // static
{
    // used by code generation 
    assert( x0 == 0.f );
    assert( y0 == 0.f );
    assert( z0 == 0.f );
    assert( z1 == 0.f );
    assert( w1 == 0.f );


    float radius_ = w0 ; 
    float z1_ = x1 ; 
    float z2_ = y1 ; 

    return Create( radius_, z1_, z2_, false );
}






float ncylinder::x() const { return param.f.x ; }
float ncylinder::y() const { return param.f.y ; }
float ncylinder::z() const { return 0.f ; }   // <--- where is this used ? z1 z2 
float ncylinder::radius() const { return param.f.w ; }
float ncylinder::r1()     const { return param.f.w ; } // so can treat like a cone in NNodeUncoincide
float ncylinder::r2()     const { return param.f.w ; }
glm::vec3 ncylinder::center() const { return glm::vec3(x(),y(),0.f) ; }

float ncylinder::z2() const { return param1.f.y ; }
float ncylinder::z1() const { return param1.f.x ; }

// grow the cylinder upwards on upper side (z2) or downwards on down side (z1)
void  ncylinder::increase_z2(float dz)
{ 
    assert( dz >= 0.f) ; 
    float _z2 = z2(); 
    float new_z2 = _z2 + dz ; 

    LOG(info) 
        << " treeidx " << get_treeidx()
        << " _z2 " << _z2
        << " dz " << dz
        << " new_z2 " << new_z2 
        ; 

    set_z2(new_z2); 
}  

void  ncylinder::decrease_z1(float dz)
{ 
    assert( dz >= 0.f) ; 

    float _z1 = z1(); 
    float new_z1 = _z1 - dz ; 

    LOG(info) 
        << " treeidx " << get_treeidx()
        << " _z1 " << _z1
        << " dz " << dz
        << " new_z1 " << new_z1
        ; 

    set_z1( new_z1 ); 
}

void ncylinder::set_z1(float new_z1)
{
    param1.f.x = new_z1 ; 
}
void ncylinder::set_z2(float new_z2)
{
    param1.f.y = new_z2 ; 
}



nbbox ncylinder::bbox() const 
{
    float r = radius();
    glm::vec3 c = center();

    glm::vec3 mx(c.x + r, c.y + r, z2() );
    glm::vec3 mi(c.x - r, c.y - r, z1() );

    nbbox bb = make_bbox(mi, mx, complement);

    return gtransform ? bb.make_transformed(gtransform->t) : bb ; 
}

/**




**/

float ncylinder::operator()(float x_, float y_, float z_) const 
{
    glm::vec4 p(x_,y_,z_,1.0); 
    if(gtransform) p = gtransform->v * p ; 

    // distance to infinite cylinder
    float dinf = glm::distance( glm::vec2(p.x, p.y), glm::vec2(x(), y()) ) - radius() ;  // <- no z-dep

    float qcap_z = z2() ;  // typically +ve   z2>z1  
    float pcap_z = z1() ;  // typically -ve

    float d_PQCAP = fmaxf( p.z - qcap_z, -(p.z - pcap_z) );

    float sd = fmaxf( d_PQCAP, dinf );   // "CSG" intersect the slab with the cylinder 

/*
    std::cout 
          << "ncylinder" 
          << " p " << p 
          << " dinf " << dinf
          << " dcap " << dcap
          << " sd " << sd
          << std::endl 
          ;
*/
    return complement ? -sd : sd ; 
} 




glm::vec3 ncylinder::gseedcenter() const 
{
    return gtransform == NULL ? center() : glm::vec3( gtransform->t * glm::vec4(center(), 1.f ) ) ;
}

glm::vec3 ncylinder::gseeddir() const 
{
    glm::vec4 dir(1,0,0,0);   // Z: not a good choice as without endcap fail to hit 
    if(gtransform) dir = gtransform->t * dir ; 
    return glm::vec3(dir) ;
}

void ncylinder::pdump(const char* msg ) const 
{
    std::cout 
              << std::setw(10) << msg 
              << " label " << ( label ? label : "no-label" )
              << " center " << center() 
              << " radius " << radius() 
              << " z1 " << z1()
              << " z2 " << z2()
              << " gseedcenter " << gseedcenter()
              << " gtransform " << !!gtransform 
              << std::endl ; 

    if(verbosity > 1 && gtransform) std::cout << *gtransform << std::endl ;
}







unsigned ncylinder::par_nsurf() const 
{
   return 3 ; 
}
int ncylinder::par_euler() const 
{
   return 2 ; 
}
unsigned ncylinder::par_nvertices(unsigned /*nu*/, unsigned /*nv*/) const 
{
   return 0 ; 
}

glm::vec3 ncylinder::par_pos_model(const nuv& uv) const 
{
    unsigned s  = uv.s(); 
    assert(s < par_nsurf());

    float r1_ = radius();
    float r2_ = radius();
    float z1_ = z1();
    float z2_ = z2();

    assert( z2_ > z1_ );

    glm::vec3 pos(0,0,0);
    pos.x = x();
    pos.y = y();
    // start on axis

    switch(s)
    {
       case 0:  ncone::_par_pos_body(  pos, uv, r1_ ,  z1_ , r2_ , z2_ ) ; break ; 
       case 1:  nnode::_par_pos_endcap(pos, uv, r2_ ,  z2_ )             ; break ; 
       case 2:  nnode::_par_pos_endcap(pos, uv, r1_ ,  z1_ )             ; break ; 
    }
    return pos ; 
}




/*

Can SDFs model finite open cylinder, ie no endcaps or 1 endcap  ?
====================================================================

* i do not think so...  but then CSG cannot do this either

* suspect this is fundamental limitation of geometry modelling with SDF,
  ... **can only handle closed geometry** 

  * yep that is definitiely the case for CSG  *S* means SOLID,


Extract from env-;sdf-:

Slab is a difference of half-spaces

* sdfA = z - h      (plane at z = h) 
* sdfB = z + h      (plane at z = -h ),  
* ~sdfB = -(z+h)    (same position, but now inside are upwards to +z)

::

    intersect(sdfA, ~sdfB) 
    max( z - h , -(z + h) )
    max( z - h , -z - h )
    max(z, -z) - h
    abs(z) - h 


http://iquilezles.org/www/articles/distfunctions/distfunctions.htm

float sdCappedCylinder( vec3 p, vec2 h )
{
  vec2 d = abs(vec2(length(p.xz),p.y)) - h;
  return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

http://mercury.sexy/hg_sdf/
http://aka-san.halcy.de/distance_fields_prefinal.pdf

By using CSG operations, we can now cut parts of the (infinite) cylinder 
by intersecting it and an infinite z-slab, resulting in a finite z-oriented 
cylinder with radius r and height h:

    d = max( sqrt(px^2+py^2) - r, |pz|-(h/2) )


* "max" corresponds to CSG intersection with z-slab (infinite in x and y)
   which is represented by 

    d = |pz| - (h/2)       <- d = 0 at  pz = +- h/2

*/


