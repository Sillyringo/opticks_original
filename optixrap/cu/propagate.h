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

#pragma once

/**0
propagate.h : propagate_to_boundary / propagate_at_boundary_geant4_style
===========================================================================

* https://bitbucket.org/simoncblyth/opticks/src/master/optixrap/cu/propagate.h

.. contents:: Table of Contents
   :depth: 2

0**/


/**1
propagate.h : propagate_to_boundary absorb/scatter/sail 
--------------------------------------------------------

* absorb 

  #. advance p.time and p.position to absorption point
  #. if BULK_REEMIT(CONTINUE) change p.direction p.polarization p.wavelength
  #. if BULK_ABSORB(BREAK)  .last_hit_triangle -1  

* scatter

  #. advance p.time and p.position to scattering point
  #. RAYLEIGH_SCATTER(CONTINUE)  change p.direction p.polarization 

* sail

  #. advance p.position p.time to boundary 
  #. sail to boundary(PASS)  
  #. consumes 3u 

Inputs:

* p.time
* p.position
* p.direction

* s.distance_to_boundary
* s.material1.x  refractive_index
* s.material1.y  absorption_length
* s.material1.z  scattering_length
* s.material1.w  reemission_prob

Outputs:

* p.time
* p.position
* p.direction
* p.wavelength
* p.polarization
* p.flags.i.x    (boundary)
* p.flags.i.w    (history)

Returns:

* BREAK(BULK_ABSORB)
* CONTINUE(BULK_REEMIT)
* CONTINUE(RAYLEIGH_SCATTER)
* PASS("SAIL")

1**/


__device__ int propagate_to_boundary( Photon& p, State& s, curandState &rng)
{
    //float speed = SPEED_OF_LIGHT/s.material1.x ;    // .x:refractive_index    (phase velocity of light in medium)
    float speed = s.m1group2.x ;  // .x:group_velocity  (group velocity of light in the material) see: opticks-find GROUPVEL

#ifdef WITH_ALIGN_DEV
#ifdef WITH_LOGDOUBLE

    float u_boundary_burn = curand_uniform(&rng) ;
    float u_scattering = curand_uniform(&rng) ;
    float u_absorption = curand_uniform(&rng) ;

    //  these two doubles brings about 100 lines of PTX with .f64
    //  see notes/issues/AB_SC_Position_Time_mismatch.rst      
    float scattering_distance = -s.material1.z*log(double(u_scattering)) ;   // .z:scattering_length
    float absorption_distance = -s.material1.y*log(double(u_absorption)) ;   // .y:absorption_length 

#elif WITH_LOGDOUBLE_ALT
    float u_boundary_burn = curand_uniform(&rng) ;
    double u_scattering = curand_uniform_double(&rng) ;
    double u_absorption = curand_uniform_double(&rng) ;

    float scattering_distance = -s.material1.z*log(u_scattering) ;   // .z:scattering_length
    float absorption_distance = -s.material1.y*log(u_absorption) ;   // .y:absorption_length 

#else
    float u_boundary_burn = curand_uniform(&rng) ;
    float u_scattering = curand_uniform(&rng) ;
    float u_absorption = curand_uniform(&rng) ;
    float scattering_distance = -s.material1.z*logf(u_scattering) ;   // .z:scattering_length
    float absorption_distance = -s.material1.y*logf(u_absorption) ;   // .y:absorption_length 
#endif

#else    // not-WITH_ALIGN_DEV
    float scattering_distance = -s.material1.z*logf(curand_uniform(&rng));   // .z:scattering_length
    float absorption_distance = -s.material1.y*logf(curand_uniform(&rng));   // .y:absorption_length
#endif

#ifdef WITH_ALIGN_DEV_DEBUG
    rtPrintf("propagate_to_boundary  u_OpBoundary:%.9g speed:%.9g s.distance_to_boundary:%.9g \n", u_boundary_burn, speed, s.distance_to_boundary );
    rtPrintf("propagate_to_boundary  u_OpRayleigh:%.9g   scattering_length(s.material1.z):%.9g scattering_distance:%.9g \n", u_scattering, s.material1.z, scattering_distance );
    rtPrintf("propagate_to_boundary  u_OpAbsorption:%.9g   absorption_length(s.material1.y):%.9g absorption_distance:%.9g \n", u_absorption, s.material1.y, absorption_distance );
#endif


    if (absorption_distance <= scattering_distance) 
    {
        if (absorption_distance <= s.distance_to_boundary) 
        {
            p.time += absorption_distance/speed ;   
            p.position += absorption_distance*p.direction;

            const float& reemission_prob = s.material1.w ; 
            float u_reemit = reemission_prob == 0.f ? 2.f : curand_uniform(&rng);  // avoid consumption at absorption when not scintillator

            if (u_reemit < reemission_prob)    
            {
                // no materialIndex input to reemission_lookup as both scintillators share same CDF 
                // non-scintillators have zero reemission_prob
                p.wavelength = reemission_lookup(curand_uniform(&rng));
                p.direction = uniform_sphere(&rng);
                p.polarization = normalize(cross(uniform_sphere(&rng), p.direction));
                p.flags.i.x = 0 ;   // no-boundary-yet for new direction

                s.flag = BULK_REEMIT ;
                return CONTINUE;
            }                           
            else 
            {
                s.flag = BULK_ABSORB ;
                return BREAK;
            }                         
        }
        //  otherwise sail to boundary  
    }
    else 
    {
        if (scattering_distance <= s.distance_to_boundary) 
        {
            p.time += scattering_distance/speed ; 
            p.position += scattering_distance*p.direction;

            //rayleigh_scatter(p, rng);
            rayleigh_scatter_align(p, rng);   // consumes 5u at each turn of the loop

            s.flag = BULK_SCATTER;
            p.flags.i.x = 0 ;  // no-boundary-yet for new direction

            return CONTINUE;
        } 
        //  otherwise sail to boundary  
         
    }     // if scattering_distance < absorption_distance


    p.position += s.distance_to_boundary*p.direction;
    p.time += s.distance_to_boundary/speed ;   // .x:refractive_index

    return PASS;

} // propagate_to_boundary




/**2
propagate.h : propagate_at_boundary_geant4_style
-------------------------------------------------

See env-/g4op-/G4OpBoundaryProcess.cc annotations to follow this
and compare the Opticks and Geant4 implementations.

Inputs:

* p.direction
* p.polarization

* s.material1.x    : refractive index 
* s.material2.x    : refractive index
* s.surface_normal 

Outputs:

* p.direction
* p.polarization
* p.flags.i.x     (boundary) 
* p.flags.i.w     (history)

Tacitly returns CONTINUE

Notes:

*  when geometry dictates TIR there is no dependence on u_reflect, 
   just always get reflection


fresnel reflect/transmit conventional directions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

                    s1
                  +----+          
                   \   .   /      ^
              c1   i\  .  / r    /|\
                     \ . /        |                      
        material1     \./         | n
        ---------------+----------+----------
        material2      .\
                       . \
                  c2   .  \ t
                       .   \
                       +----+
                         s2

i, incident photons 
   pointing down to interface (from material1 towards material2)

n, surface normal (s.surface_normal)
   pointing up from interface (from material2 back into material1)
   Orientation is arranged by flipping geometric normal 
   based on photon direction.

t, transmitted photons
   from interface into material2

r, reflected photons
   from interface back into material1

c1, costheta_1 
   cosine of incident angle,  c1 = dot(-i, n) = - dot(i, n)
   arranged to be positive via normal flipping 
   and corresponding flip of which material is labelled 1 and 2 
    

polarisation
~~~~~~~~~~~~~~

::
                   
   S polarized : E field perpendicular to plane of incidence
   P polarized : E field within plane of incidence 


normal incidence photons
~~~~~~~~~~~~~~~~~~~~~~~~~~~

* no unique plane of incidence, 
* artifically setting incident_plane_normal to initial p.polarisation yields normal_coefficient = 1.0f 
  so always treated as S polarized 
  
initial momentum dir
     -s.surface_normal 

final momentum dir (c1 = 1.f)
     -s.surface_normal + 2.0f*c1*s.surface_normal  = +s.surface_normal = -p.direction 
                                                   

minimise use of trancendental functions 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Obtain c2c2 from Snells Law without lots of expensive function calls.::
 
       n1 s1 = n2 s2

          s2 = eta * s1       eta = n1/n2

        c2c2 = 1 - s2s2 
             = 1 - eta eta s1 s1  
             = 1 - eta eta (1 - c1c1) 


        c2c2 - 1 = (c1c1 - 1) eta eta


TIR : total internal reflection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Total internal reflection, occurs when c2c2 < 0.f  (c2 imaginary)

Handled by: 

* artificially setting c2 = 0.f 
* results in reflection_coefficient = 1.0f so will always reflect for both S and P cases

2**/


__device__ void propagate_at_boundary_geant4_style( Photon& p, State& s, curandState &rng)
{
    const float n1 = s.material1.x ;
    const float n2 = s.material2.x ;   
    const float eta = n1/n2 ; 

    const float c1 = -dot(p.direction, s.surface_normal ); // c1 arranged to be +ve  (G4 "cost1") 
    const float eta_c1 = eta * c1 ; 

    const float c2c2 = 1.f - eta*eta*(1.f - c1 * c1 ) ;   // Snells law 
     
    bool tir = c2c2 < 0.f ; 
    const float EdotN = dot(p.polarization , s.surface_normal ) ;  // used for TIR polarization

    const float c2 = tir ? 0.f : sqrtf(c2c2) ;   // c2 chosen +ve, set to 0.f for TIR => reflection_coefficient = 1.0f : so will always reflect

    const float n1c1 = n1*c1 ; 
    const float n2c2 = n2*c2 ; 
    const float n2c1 = n2*c1 ; 
    const float n1c2 = n1*c2 ; 

    const float3 A_trans = fabs(c1) > 0.999999f ? p.polarization : normalize(cross(p.direction, s.surface_normal)) ;
    
    // decompose p.polarization onto incident orthogonal basis

    const float E1_perp = dot(p.polarization, A_trans);   // fraction of E vector perpendicular to plane of incidence, ie S polarization
    const float3 E1pp = E1_perp * A_trans ;               // S-pol transverse component   
    const float3 E1pl = p.polarization - E1pp ;           // P-pol parallel component 
    const float E1_parl = length(E1pl) ;
  
    // G4OpBoundaryProcess at normal incidence, mentions Jackson and uses 
    //      A_trans  = OldPolarization; E1_perp = 0. E1_parl = 1. 
    // but that seems inconsistent with the above dot product, above is swapped cf that

    const float E2_perp_t = 2.f*n1c1*E1_perp/(n1c1+n2c2);  // Fresnel S-pol transmittance
    const float E2_parl_t = 2.f*n1c1*E1_parl/(n2c1+n1c2);  // Fresnel P-pol transmittance

    const float E2_perp_r = E2_perp_t - E1_perp;           // Fresnel S-pol reflectance
    const float E2_parl_r = (n2*E2_parl_t/n1) - E1_parl ;  // Fresnel P-pol reflectance

    const float2 E2_t = make_float2( E2_perp_t, E2_parl_t ) ;
    const float2 E2_r = make_float2( E2_perp_r, E2_parl_r ) ;

    const float  E2_total_t = dot(E2_t,E2_t) ; 

    const float2 T = normalize(E2_t) ; 
    const float2 R = normalize(E2_r) ; 

    const float TransCoeff =  tir ? 0.0f : n2c2*E2_total_t/n1c1 ; 
    //  above 0.0f was until 2016/3/4 incorrectly a 1.0f 
    //  resulting in TIR yielding BT where BR is expected


#ifdef WITH_REFLECT_CHEAT_DEBUG
    // Debugging trick that increases "accidental" history alignment in non-aligned running. 
    // Enabling this also requires "--reflectcheat" option to set s.ureflectcheat to a fraction 
    // from 0 to 1 according to photon record_id 
    const float u_reflect = s.ureflectcheat >= 0.f ? s.ureflectcheat : curand_uniform(&rng) ;
    bool reflect = u_reflect > TransCoeff  ;
#else
    const float u_reflect = curand_uniform(&rng) ;
    bool reflect = u_reflect > TransCoeff  ;
#endif 

#ifdef WITH_ALIGN_DEV_DEBUG
    rtPrintf("propagate_at_boundary  u_OpBoundary_DiDiTransCoeff:%.9g  n_reflect:%d   c_TransCoeff:%.9g  c2c2:%10.4f n_tir:%d  post (%10.4f %10.4f %10.4f %10.4f) pol (%10.4f %10.4f %10.4f ) \n",
         u_reflect, reflect, TransCoeff, c2c2, tir, p.position.x, p.position.y, p.position.z, p.time, p.polarization.x, p.polarization.y, p.polarization.z  );
#endif

    p.direction = reflect 
                    ? 
                       p.direction + 2.0f*c1*s.surface_normal 
                    : 
                       eta*p.direction + (eta_c1 - c2)*s.surface_normal
                    ;   

    const float3 A_paral = normalize(cross(p.direction, A_trans));

    p.polarization = reflect ?
                                ( tir ? 
                                        -p.polarization + 2.f*EdotN*s.surface_normal 
                                      :
                                        R.x*A_trans + R.y*A_paral 
                                )
                             :
                                T.x*A_trans + T.y*A_paral 
                             ;


    s.flag = reflect     ? BOUNDARY_REFLECT : BOUNDARY_TRANSMIT ; 

    p.flags.i.x = 0 ;  // no-boundary-yet for new direction
}



__device__ void UNUSED_DEAD_CODE_propagate_at_boundary( Photon& p, State& s, curandState &rng)
{
    float eta = s.material1.x/s.material2.x ;    // eta = n1/n2   x:refractive_index  PRE-FLIPPED

    const float c1 = -dot(p.direction, s.surface_normal ); // c1 arranged to be +ve   

    float3 incident_plane_normal = fabs(c1) < 1e-6f ? p.polarization : normalize(cross(p.direction, s.surface_normal)) ;

    float normal_coefficient = dot(p.polarization, incident_plane_normal);  // fraction of E vector perpendicular to plane of incidence, ie S polarization

    const float c2c2 = 1.f - eta*eta*(1.f - c1 * c1 ) ; 

    bool tir = c2c2 < 0.f ; 

    const float c2 = tir ? 0.f : sqrtf(c2c2) ;   // c2 chosen +ve, set to 0.f for TIR => reflection_coefficient = 1.0f : so will always reflect

    const float eta_c1 = eta * c1 ; 

    const float eta_c2 = eta * c2 ;    

    bool s_polarized = curand_uniform(&rng) < normal_coefficient*normal_coefficient ;

    const float reflection_coefficient = s_polarized 
                      ? 
                         (eta_c1 - c2)/(eta_c1 + c2 )  
                      :
                         (c1 - eta_c2)/(c1 + eta_c2)  
                      ; 

    bool reflect = curand_uniform(&rng) < reflection_coefficient*reflection_coefficient ;

    // need to find new direction first as polarization depends on it for case P

    p.direction = reflect 
                    ? 
                       p.direction + 2.0f*c1*s.surface_normal 
                    : 
                       eta*p.direction + (eta_c1 - c2)*s.surface_normal
                    ;   

    p.polarization = s_polarized 
                       ? 
                          incident_plane_normal
                       :
                          normalize(cross(incident_plane_normal, p.direction))
                       ;
    

    //p.flags.i.w |= reflect     ? BOUNDARY_REFLECT : BOUNDARY_TRANSMIT ;
    //p.flags.i.w |= s_polarized ? BOUNDARY_SPOL    : BOUNDARY_PPOL ;
    //p.flags.i.w |= tir         ? BOUNDARY_TIR     : BOUNDARY_TIR_NOT ; 

    s.flag = reflect     ? BOUNDARY_REFLECT : BOUNDARY_TRANSMIT ; 

    p.flags.i.x = 0 ;  // no-boundary-yet for new direction
}



/**3
propagate.h : propagate_at_specular_reflector / propagate_at_specular_reflector_geant4_style
----------------------------------------------------------------------------------------------

Inputs:

* p.direction
* p.polarization

* s.surface_normal
* s.cos_theta

Outputs:

* p.direction
* p.polarization
* p.flags.i.x
* p.flags.i.w

Returns:

CONTINUE

3**/

//#define DEBUG_POLZ 1

__device__ void propagate_at_specular_reflector(Photon &p, State &s, curandState &rng)
{
    const float c1 = -dot(p.direction, s.surface_normal );     
    //   c1 +ve for directions opposite to normal, (ie from outside) 
    //   c1 -ve for directions same as normal (ie from inside)
 
    // see notes/issues/photon-polarization-testauto-SR.rst

    float3 incident_plane_normal = fabs(c1) > 0.999999f ? p.polarization : normalize(cross(p.direction, s.surface_normal)) ;

#ifdef DEBUG_POLZ
    rtPrintf("// propagate_at_specular_reflector.0 dir (%10.4f %10.4f %10.4f) \n", p.direction.x, p.direction.y, p.direction.z ); 
    rtPrintf("// propagate_at_specular_reflector.0 snorm (%10.4f %10.4f %10.4f) \n", s.surface_normal.x, s.surface_normal.y, s.surface_normal.z ); 
    rtPrintf("// propagate_at_specular_reflector.0 polz (%10.4f %10.4f %10.4f) \n", p.polarization.x, p.polarization.y, p.polarization.z ); 
    rtPrintf("// propagate_at_specular_reflector.0 s.ct c1 (%10.4f %10.4f ) \n", s.cos_theta, c1 ); 
    rtPrintf("// propagate_at_specular_reflector.0 ipn (%10.4f %10.4f %10.4f) \n", incident_plane_normal.x, incident_plane_normal.y, incident_plane_normal.z ); 
#endif


    float normal_coefficient = dot(p.polarization, incident_plane_normal);  // fraction of E vector perpendicular to plane of incidence, ie S polarization

    p.direction += 2.0f*c1*s.surface_normal  ;  

    bool s_polarized = curand_uniform(&rng) < normal_coefficient*normal_coefficient ;

    p.polarization = s_polarized 
                       ? 
                          incident_plane_normal
                       :
                          normalize(cross(incident_plane_normal, p.direction))
                       ;

    p.flags.i.x = 0 ;  // no-boundary-yet for new direction

#ifdef DEBUG_POLZ
    //rtPrintf("// propagate_at_specular_reflector dir (%10.4f %10.4f %10.4f) \n", p.direction.x, p.direction.y, p.direction.z ); 
    //rtPrintf("// propagate_at_specular_reflector snorm (%10.4f %10.4f %10.4f) \n", s.surface_normal.x, s.surface_normal.y, s.surface_normal.z ); 
    rtPrintf("// propagate_at_specular_reflector.1 polz (%10.4f %10.4f %10.4f) \n", p.polarization.x, p.polarization.y, p.polarization.z ); 
#endif
} 


__device__ void propagate_at_specular_reflector_geant4_style(Photon &p, State &s, curandState &rng)
{
    // NB no-s-pol throwing 

    const float c1 = -dot(p.direction, s.surface_normal );      // G4double PdotN = OldMomentum * theFacetNormal;

    float normal_coefficient = dot(p.polarization, s.surface_normal);    // G4double EdotN = OldPolarization * theFacetNormal;
    // EdotN : fraction of E vector perpendicular to plane of incidence, ie S polarization

    p.direction += 2.0f*c1*s.surface_normal  ;  

    p.polarization = -p.polarization + 2.f*normal_coefficient*s.surface_normal  ;  // NewPolarization = -OldPolarization + (2.*EdotN)*theFacetNormal;

    p.flags.i.x = 0 ;  // no-boundary-yet for new direction
}


/*

    311 inline
    312 void DsG4OpBoundaryProcess::DoReflection()
    313 {
    ...
    328         else {
    329 
    330           theStatus = SpikeReflection;
    331           theFacetNormal = theGlobalNormal;
    332           G4double PdotN = OldMomentum * theFacetNormal;
    333           NewMomentum = OldMomentum - (2.*PdotN)*theFacetNormal;
    334 
    335         }
    336         G4double EdotN = OldPolarization * theFacetNormal;
    337         NewPolarization = -OldPolarization + (2.*EdotN)*theFacetNormal;
    338 }


*/


/**4
propagate.h : propagate_at_diffuse_reflector / propagate_at_diffuse_reflector_geant4_style
--------------------------------------------------------------------------------------------

Changes p.direction, p.polarization and p.flags.i.x

4**/

__device__ void propagate_at_diffuse_reflector(Photon &p, State &s, curandState &rng)
{
    float ndotv;
    do {
	    p.direction = uniform_sphere(&rng);
	    ndotv = dot(p.direction, s.surface_normal);
	    if (ndotv < 0.0f) 
        {
	        p.direction = -p.direction;
	        ndotv = -ndotv;
	    }
    } while (! (curand_uniform(&rng) < ndotv) );

    p.polarization = normalize( cross(uniform_sphere(&rng), p.direction));
    p.flags.i.x = 0 ;  // no-boundary-yet for new direction
}                       




__device__ void propagate_at_diffuse_reflector_geant4_style(Photon &p, State &s, curandState &rng)
{
    float3 old_direction = p.direction ; 

    float ndotv;
    do {
	    p.direction = uniform_sphere(&rng);
	    ndotv = dot(p.direction, s.surface_normal);
	    if (ndotv < 0.0f) 
        {
	        p.direction = -p.direction;
	        ndotv = -ndotv;
	    }
    } while (! (curand_uniform(&rng) < ndotv) );


    float3 facet_normal = normalize( p.direction - old_direction ) ;

    float normal_coefficient = dot(p.polarization, facet_normal);  // EdotN

    p.polarization = -p.polarization + 2.f*normal_coefficient*facet_normal ; 

    p.flags.i.x = 0 ;  // no-boundary-yet for new direction
} 



/**5
propagate.h : propagate_at_surface
------------------------------------

Inputs:

* s.surface.x detect
* s.surface.y absorb              (1.f - reflectivity ) ?
* s.surface.z reflect_specular
* s.surface.w reflect_diffuse

These properties are setup in GSurfaceLib::createStandardSurface
which then get interveaved with material properties into the boundary_texture 
for GPU access.

Surface and material properties are read from the texture for the
relevant boundary index that results from the ray trace. 

* optixrap/cu/state.h:fill_state 
* optixrap/cu/boundary_lookup.h 

Returns:

* BREAK(SURFACE_ABSORB) 
* BREAK(SURFACE_DETECT) 
* CONTINUE(SURFACE_DREFLECT) 
* CONTINUE(SURFACE_SREFLECT) 

Known differences vs counterpart DsG4OpBoundaryProcess::DoAbsorption
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* u_surface_burn not compared against theEfficiency to decide on detect 


TODO
~~~~~~

* arrange values to do equivalent to G4 ?

   absorb + detect + reflect_diffuse + reflect_specular  = 1   ??

* How to handle special casing of some surfaces...

  * SPECULARLOBE...


5**/

__device__ int
propagate_at_surface(Photon &p, State &s, curandState &rng)
{
    float u_surface = curand_uniform(&rng);
#ifdef WITH_ALIGN_DEV
    float u_surface_burn = curand_uniform(&rng);  
#endif

#ifdef WITH_ALIGN_DEV_DEBUG
    rtPrintf("propagate_at_surface   u_OpBoundary_DiDiAbsorbDetectReflect:%.9g \n", u_surface);
    rtPrintf("propagate_at_surface   u_OpBoundary_DoAbsorption:%.9g \n", u_surface_burn);
#endif

    if( u_surface < s.surface.y )   // absorb   
    {
        s.flag = SURFACE_ABSORB ;
        s.index.x = s.index.y ;   // kludge to get m2 into seqmat for BREAKERs
        return BREAK ;
    }
    else if ( u_surface < s.surface.y + s.surface.x )  // absorb + detect
    {
        s.flag = SURFACE_DETECT ;
        s.index.x = s.index.y ;   // kludge to get m2 into seqmat for BREAKERs
        return BREAK ;
    } 
    else if (u_surface  < s.surface.y + s.surface.x + s.surface.w )  // absorb + detect + reflect_diffuse 
    {
        s.flag = SURFACE_DREFLECT ;
        propagate_at_diffuse_reflector_geant4_style(p, s, rng);
        return CONTINUE;
    }
    else
    {
        s.flag = SURFACE_SREFLECT ;
        //propagate_at_specular_reflector(p, s, rng );
        propagate_at_specular_reflector_geant4_style(p, s, rng );
        return CONTINUE;
    }
}


