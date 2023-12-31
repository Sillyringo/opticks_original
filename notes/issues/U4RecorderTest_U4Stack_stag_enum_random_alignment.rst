U4RecorderTest_U4Stack_stag_enum_random_alignment
===================================================


Overview
---------

* Next: :doc:`higher_stats_U4RecorderTest_cxs_rainbow_random_aligned_comparison`

Opticks
   qsim.h stagr.h stag.h 
Geant4 
   U4Recorder U4Stack::Classify SBacktrace.h  

Machinery for enumerating and collecting random consumption records 
in both contexts is essentially complete following the ideas from prior. 

* :doc:`ideas_on_random_alignment_in_new_workflow`

Note that if there is a need to annotate the "simstreams" to help with
alignment : can do that simply by adding burns with suitable enum names. 

Need to apply the machinery to input_photons with a variety of
propagation histories to observe the consumption patterns
in order to decide how best to align. 


Summary
---------

Progress continues steadily.  

To develop+test the new workflow validation machinery I have been doing
random aligned comparisons with very simple geometry and input photons. 
Using input photons avoids having to random align the generation as both 
contexts can run from the same input photons.  

Boundary reflect/transmit histories are matching in low statistics checks. 
The next step is to increase the statistics and increase the size of the 
simple geometry in order to get absorption and scattering aligned
and then to run input photon validations in more complex geometries building 
up to the full geometry.  

Because of the very different ways of implementing reemission (and hence 
very different pattern of random consumption) I expect aligning reemission will 
be a lot more difficult than boundary/absorption/scattering.

Nevertheless I will have a quick attempt at aligning reemission 
to see in detail what the difficulties are.

Getting the simulations to run aligned is a significant effort, but
is has a huge advantage of then making validation and geometry issue finding 
very easy as direct comparisons unclouded by statistics are then possible. 
 
Of course input photon tests can be setup starting within the water 
which do not need reemission to be aligned, so aligning reemission while convenient
is not essential. 

If it looks to be too time consuming to align reemission I will proceed with 
bringing the statistical level comparison python machinery over to work 
with the new workflow event arrays. 

The statistical comparison machinery is needed anyhow as random aligned 
running beyond ~1M will find very rare issues that are not significant 
to real running. 



WIP : apply consumption enum collection machinery with storch_test.sh input photons
-----------------------------------------------------------------------------------------

::

    cx
    ./cxs_raindrop.sh       # remote 
    ./cxs_raindrop.sh grab  # local 

    u4t
    ./U4RecorderTest.sh     # local 

    ## NB without the below envvar U4RecorderTest does not fill the stack tags, leaving them all zero
    ##  export U4Random_flat_debug=1  

    u4t
    ./U4RecorderTest_ab.sh     # local 
     



TODO : observe with bigger geometry so can see AB and SC in the history 
--------------------------------------------------------------------------



TODO : investigate impact of U4Process::ClearNumberOfInteractionLengthLeft 
-----------------------------------------------------------------------------

Q: U4Process::ClearNumberOfInteractionLengthLeft will inevitably change the simulation because are using 
   different randoms, but does it change the correctness of the simulation ?

A: Assuming just technical change, because the chances of SC/AB etc..
   are surely independent of what happened before ? 

To verify the assumption need high stats statistical comparison of history frequencies 
with and without this trick being applied. 
This will require getting the statistical comparison python machinery into new workflow
using the new SEvt arrays.  


DONE : check again after sctx rejig
-------------------------------------------------

::

    In [4]: ab_photon = np.abs(a.photon - b.photon)

    In [5]: ab_photon.max()
    Out[5]: 0.0018196106

    In [6]: ab_record = np.abs(a.record - b.record)
    In [7]: ab_record.max()
    Out[7]: 0.0018196106


DONE : formalized such comparisons using eprint/epr 


DONE : direct photon + record step point comparison  
------------------------------------------------------

::

    In [7]: ab_photon = np.abs(a.photon - b.photon)

    In [10]: ab_photon.max()
    Out[10]: 0.0018196106

    In [16]: ab_record = np.abs(a.record - b.record)
    In [17]: ab_record.max()
    Out[17]: 0.0018196106

    In [8]: np.where(ab_photon > 1e-5)
    Out[8]: 
    (array([ 3,  3, 15, 18, 18, 20, 25, 26, 33, 36, 38, 42, 49, 51, 54, 54, 55, 63, 66, 69, 69, 72, 72, 75, 75, 75, 78, 87, 94, 98]),
     array([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]),
     array([0, 1, 2, 0, 1, 1, 2, 1, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 0, 0, 0, 0]))

    In [18]: np.where(ab_record > 1e-5)
    Out[18]: 
    (array([ 0,  3,  3, 12, 13, 15, 18, 18, 20, 25, 26, 30, 33, 36, 36, 38, 42, 44, 49, 51, 53, 54, 54, 54, 54, 55, 57, 63, 66, 69, 69, 70, 72, 72, 75, 75, 75, 78, 84, 87, 90, 94, 95, 98]),
     array([2, 2, 2, 2, 2, 2, 3, 3, 3, 2, 3, 2, 3, 3, 4, 3, 3, 2, 3, 3, 3, 3, 4, 5, 5, 3, 2, 3, 3, 3, 3, 2, 3, 3, 2, 2, 2, 3, 2, 3, 2, 3, 2, 3]),
     array([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]),
     array([2, 0, 1, 2, 2, 2, 0, 1, 1, 2, 1, 2, 0, 2, 0, 0, 0, 2, 1, 0, 2, 0, 2, 0, 1, 1, 2, 0, 1, 0, 1, 2, 0, 1, 0, 1, 2, 0, 2, 0, 2, 0, 2, 0]))

    ## the biggest differences are in the positions of step points 2 or 3 : thats probably the endpoint 


    In [13]: ab_photon[ab_photon > 1e-5]*1000.
    Out[13]: 
    array([0.053, 0.023, 0.045, 0.021, 0.023, 0.011, 0.023, 0.01 , 0.015, 0.031, 0.012, 0.01 , 0.017, 0.017, 0.046, 0.027, 0.01 , 0.011, 0.012, 0.015, 0.015, 0.021, 0.034, 0.233, 0.259, 1.82 , 0.013,
           0.01 , 0.013, 0.01 ], dtype=float32)

    In [21]:  ab_photon[ab_photon > 1e-4]*1000.
    Out[21]: array([0.233, 0.259, 1.82 ], dtype=float32)

    In [22]: np.where(ab_photon > 1e-4)
    Out[22]: (array([75, 75, 75]), array([0, 0, 0]), array([0, 1, 2]))

    In [23]: np.where(ab_record > 1e-4)
    Out[23]: (array([75, 75, 75]), array([2, 2, 2]), array([0, 0, 0]), array([0, 1, 2]))


    In [26]: a.record[75,:4]
    Out[26]: 
    array([[[-20.457,  22.904, -90.   ,   0.   ],
            [  0.   ,   0.   ,   1.   ,   0.   ],
            [  0.746,   0.666,   0.   , 501.   ],
            [  0.   ,   0.   ,   0.   ,   0.   ]],

           [[-20.457,  22.904, -39.458,   0.169],
            [  0.125,  -0.14 ,   0.982,   0.   ],
            [  0.746,   0.666,  -0.   , 501.   ],
            [  0.   ,   0.   ,  -0.   ,   0.   ]],

           [[-16.643,  18.634,  -9.458,   0.31 ],
            [  0.125,  -0.14 ,   0.982,   0.   ],
            [  0.746,   0.666,  -0.   , 501.   ],
            [  0.   ,   0.   ,   0.   ,   0.   ]],

           [[  0.   ,   0.   ,   0.   ,   0.   ],
            [  0.   ,   0.   ,   0.   ,   0.   ],
            [  0.   ,   0.   ,   0.   ,   0.   ],
            [  0.   ,   0.   ,   0.   ,   0.   ]]], dtype=float32)

    In [27]: b.record[75,:4]
    Out[27]: 
    array([[[-20.457,  22.904, -90.   ,   0.   ],
            [  0.   ,   0.   ,   1.   ,   0.   ],
            [  0.746,   0.666,   0.   , 501.   ],
            [  0.   ,   0.   ,   0.   ,   0.   ]],

           [[-20.457,  22.904, -39.458,   0.169],
            [  0.125,  -0.14 ,   0.982,   0.   ],
            [  0.746,   0.666,  -0.   , 501.   ],
            [  0.   ,   0.   ,   0.   ,   0.   ]],

           [[-16.643,  18.634,  -9.456,   0.31 ],
            [  0.125,  -0.14 ,   0.982,   0.   ],
            [  0.746,   0.666,  -0.   , 501.   ],
            [  0.   ,   0.   ,   0.   ,   0.   ]],

           [[  0.   ,   0.   ,   0.   ,   0.   ],
            [  0.   ,   0.   ,   0.   ,   0.   ],
            [  0.   ,   0.   ,   0.   ,   0.   ],
            [  0.   ,   0.   ,   0.   ,   0.   ]]], dtype=float32)

    In [28]: seqhis_(a.seq[75,0])
    Out[28]: 'TO BT AB'

    In [29]: seqhis_(b.seq[75,0])
    Out[29]: 'TO BT AB'

    In [31]: np.where(a.seq[:,0] == 1229)
    Out[31]: (array([75]),)

    In [32]: np.where(b.seq[:,0] == 1229)
    Out[32]: (array([75]),)


* largest difference from the position of the only AB:BULK_ABSORB photon in the 100

::

    In [33]: ats[75]
    Out[33]: 
    array([[1, 2, 3, 4, 5, 6, 0, 0, 0, 0],
           [1, 2, 3, 4, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8)

    In [34]: bts[75]
    Out[34]: 
    array([[2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8)

    In [35]: afs[75]
    Out[35]: 
    array([[0.373, 0.854, 0.038, 0.268, 0.974, 0.59 , 0.   , 0.   , 0.   , 0.   ],
           [0.297, 0.226, 0.922, 0.999, 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ]], dtype=float32)

    In [36]: bfs[75]
    Out[36]: 
    array([[0.373, 0.854, 0.038, 0.268, 0.974, 0.59 , 0.   , 0.   , 0.   , 0.   ],
           [0.297, 0.226, 0.922, 0.999, 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ]], dtype=float32)


sysrap/xfold.sh simplify enum label dumping using opticks.sysrap.xfold::

    In [2]: B(75)
    Out[2]: 
    B(75) : TO BT AB
     0 :     0.3727 :  2 : ScintDiscreteReset :  
     1 :     0.8539 :  6 : BoundaryDiscreteReset :  
     2 :     0.0380 :  4 : RayleighDiscreteReset :  
     3 :     0.2685 :  3 : AbsorptionDiscreteReset :  
     4 :     0.9740 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :  
     5 :     0.5896 :  7 : BoundaryDiDiTransCoeff :  

     6 :     0.2975 :  2 : ScintDiscreteReset :  
     7 :     0.2261 :  6 : BoundaryDiscreteReset :  
     8 :     0.9222 :  4 : RayleighDiscreteReset :  
     9 :     0.9992 :  3 : AbsorptionDiscreteReset :  
    10 :     0.0000 :  0 : Unclassified :  
    11 :     0.0000 :  0 : Unclassified :  

    In [3]: A(75)
    Out[3]: 
    A(75) : TO BT AB
     0 :     0.3727 :  1 :     to_sci : qsim::propagate_to_boundary u_to_sci burn 
     1 :     0.8539 :  2 :     to_bnd : qsim::propagate_to_boundary u_to_bnd burn 
     2 :     0.0380 :  3 :     to_sca : qsim::propagate_to_boundary u_scattering 
     3 :     0.2685 :  4 :     to_abs : qsim::propagate_to_boundary u_absorption 
     4 :     0.9740 :  5 :    at_burn : boundary burn 
     5 :     0.5896 :  6 :     at_ref : u_reflect > TransCoeff 

     6 :     0.2975 :  1 :     to_sci : qsim::propagate_to_boundary u_to_sci burn 
     7 :     0.2261 :  2 :     to_bnd : qsim::propagate_to_boundary u_to_bnd burn 
     8 :     0.9222 :  3 :     to_sca : qsim::propagate_to_boundary u_scattering 
     9 :     0.9992 :  4 :     to_abs : qsim::propagate_to_boundary u_absorption 
    10 :     0.0000 :  0 :      undef : undef 
    11 :     0.0000 :  0 :      undef : undef 




DONE : see if a 1-to-1 mapping from stack to tag can work (or vv) 
---------------------------------------------------------------------

The A:tag and B:stack do not match of course : they are different enumerations. 

A:tag
    are very specific corresponding to a curand_uniform call followed by tagr.add
B:stack
    correspond to backtraces 

Going from more specific to less A:tag->B:stack is the easier mapping direction.

Is is possible to find a 1-to-1 mapping between the A:tag and B:stack::

    In [10]: ats[0]
    Out[10]: 
    array([[1, 2, 3, 4, 5, 6, 0, 0, 0, 0],
           [1, 2, 3, 4, 5, 6, 0, 0, 0, 0],
           [1, 2, 3, 4, 7, 8, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8)

    In [11]: bts[0]
    Out[11]: 
    array([[2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 9, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8)


Where mapping values::

    In [22]: ats0 = ats[0].copy() ; ats0 
    Out[22]: 
    array([[1, 2, 3, 4, 5, 6, 0, 0, 0, 0],
           [1, 2, 3, 4, 5, 6, 0, 0, 0, 0],
           [1, 2, 3, 4, 7, 8, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8)

    In [24]: np.where( ats0 == 1 )
    Out[24]: (array([0, 1, 2]), array([0, 0, 0]))

    In [26]: ats0[np.where( ats0 == 1 )] = 10 ; ats0
    Out[26]: 
    array([[10,  2,  3,  4,  5,  6,  0,  0,  0,  0],
           [10,  2,  3,  4,  5,  6,  0,  0,  0,  0],
           [10,  2,  3,  4,  7,  8,  0,  0,  0,  0],
           [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0]], dtype=uint8)


See U4Stack.py::

    In [4]: stack.tag2stack
    Out[4]: 
    OrderedDict([(0, 0),
                 (1, 2),
                 (2, 6),
                 (3, 4),
                 (4, 3),
                 (5, 8),
                 (6, 7),
                 (7, 8),
                 (8, 9),
                 (9, 0),
                 (10, 0),
                 (11, 0),
                 (12, 0),
                 (13, 0),
                 (14, 0),
                 (15, 0),
                 (16, 0),
                 (17, 0),
                 (18, 0),
                 (19, 0),
                 (20, 0),
                 (21, 0),
                 (22, 0),
                 (23, 6),
                 (24, 4),
                 (25, 3)])

    In [5]: stack.stack2tag
    Out[5]: 
    OrderedDict([(0, 22),
                 (2, 1),
                 (6, 23),
                 (4, 24),
                 (3, 25),
                 (8, 7),
                 (7, 6),
                 (9, 8)])


* HMM: the above looks like argument to get rid of the 22,23,24,25 for the post-BR/StepTooSmall burns
  as they introduce complication of breaking 1-to-1

* done this, but still not 1-to-1 because of BoundaryBurn_SurfaceReflectTransmitAbsorb

::

    U4Stack.py:dump_tag2stack
     1 :     to_sci : qsim::propagate_to_boundary u_to_sci burn                      :  2 : ScintDiscreteReset :                                                        
     2 :     to_bnd : qsim::propagate_to_boundary u_to_bnd burn                      :  6 : BoundaryDiscreteReset :                                                     
     3 :     to_sca : qsim::propagate_to_boundary u_scattering                       :  4 : RayleighDiscreteReset :                                                     
     4 :     to_abs : qsim::propagate_to_boundary u_absorption                       :  3 : AbsorptionDiscreteReset :                                                   

     5 :    at_burn : boundary burn                                                  :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :                                 
     6 :     at_ref : u_reflect > TransCoeff                                         :  7 : BoundaryDiDiTransCoeff :                                                    
     7 :      sf_sd : qsim::propagate_at_surface ab/sd                               :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :                                 
     8 :    sf_burn : qsim::propagate_at_surface burn                                :  9 : AbsorptionEffDetect :                                                       

    U4Stack.py:dump_stack2tag
     2 : ScintDiscreteReset :                                                        :  1 :     to_sci : qsim::propagate_to_boundary u_to_sci burn                      
     6 : BoundaryDiscreteReset :                                                     :  2 :     to_bnd : qsim::propagate_to_boundary u_to_bnd burn                      
     4 : RayleighDiscreteReset :                                                     :  3 :     to_sca : qsim::propagate_to_boundary u_scattering                       
     3 : AbsorptionDiscreteReset :                                                   :  4 :     to_abs : qsim::propagate_to_boundary u_absorption                       

     8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :                                 :  7 :      sf_sd : qsim::propagate_at_surface ab/sd                               
     7 : BoundaryDiDiTransCoeff :                                                    :  6 :     at_ref : u_reflect > TransCoeff                                         
     9 : AbsorptionEffDetect :                                                       :  8 :    sf_burn : qsim::propagate_at_surface burn                                


Still not 1-to-1 as both stag_at_burn+stag_sf_sd map to BoundaryBurn_SurfaceReflectTransmitAbsorb

* this reflects that separate methods handle surface and boundary in Opticks but one method does that in Geant4 
* DONE : use a common stag for these, making the mapping 1-to-1  

* DONE: reorder U4Stack to make the mapping simpler BUT offset to make it clear they are different enum 
 

U4Stack.py:dump_tag2stack::

    00 :      undef : undef                                                          :  0 : Unclassified :                                                              
     1 :     to_sci : qsim::propagate_to_boundary u_to_sci burn                      :  2 : ScintDiscreteReset :                                                        
     2 :     to_bnd : qsim::propagate_to_boundary u_to_bnd burn                      :  6 : BoundaryDiscreteReset :                                                     
     3 :     to_sca : qsim::propagate_to_boundary u_scattering                       :  4 : RayleighDiscreteReset :                                                     
     4 :     to_abs : qsim::propagate_to_boundary u_absorption                       :  3 : AbsorptionDiscreteReset :                                                   
     5 :    at_burn : boundary burn                                                  :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :                                 
     6 :     at_ref : u_reflect > TransCoeff                                         :  7 : BoundaryDiDiTransCoeff :                                                    
     7 :      sf_sd : qsim::propagate_at_surface ab/sd                               :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :                                 
     8 :    sf_burn : qsim::propagate_at_surface burn                                :  9 : AbsorptionEffDetect :                                                       
     9 :     to_ree : qsim::propagate_to_boundary u_reemit                           :  0 : Unclassified :                                                              
    10 :      re_wl : qsim::propagate_to_boundary u_wavelength                       :  0 : Unclassified :                                                              
    11 :  re_mom_ph : qsim::propagate_to_boundary re mom uniform_sphere ph           :  0 : Unclassified :                                                              
    12 :  re_mom_ct : qsim::propagate_to_boundary re mom uniform_sphere ct           :  0 : Unclassified :                                                              
    13 :  re_pol_ph : qsim::propagate_to_boundary re pol uniform_sphere ph           :  0 : Unclassified :                                                              
    14 :  re_pol_ct : qsim::propagate_to_boundary re pol uniform_sphere ct           :  0 : Unclassified :                                                              
    15 :      hp_ph : qsim::hemisphere_polarized u_hemipol_phi                       :  0 : Unclassified :                                                              
    16 :      hp_ct : qsim::hemisphere_polarized cosTheta                            :  0 : Unclassified :                                                              
    17 :      sc_u0 : qsim::rayleigh_scatter u0                                      :  0 : Unclassified :                                                              
    18 :      sc_u1 : qsim::rayleigh_scatter u1                                      :  0 : Unclassified :                                                              
    19 :      sc_u2 : qsim::rayleigh_scatter u2                                      :  0 : Unclassified :                                                              
    20 :      sc_u3 : qsim::rayleigh_scatter u3                                      :  0 : Unclassified :                                                              
    21 :      sc_u4 : qsim::rayleigh_scatter u4                                      :  0 : Unclassified :                                   



::


    210 /**
    211 U4Stack::TagToStack
    212 --------------------
    213 
    214 Attempt at mapping from A:tag to B:stack 
    215 
    216 * where to use this mapping anyhow ? unkeen to do this at C++ level as it feels like a complication 
    217   and potential info loss that is only not-info loss when are in an aligned state 
    218 
    219 * but inevitably when generalize will get out of alignment and will need to use the A:tag  
    220   and B:stack to regain alignment 
    221 
    222 * hence the right place to use the mapping is in python 
    223 
    224 **/
    225 
    226 inline unsigned U4Stack::TagToStack(unsigned tag)
    227 {
    228     unsigned stack = U4Stack_Unclassified ;
    229     switch(tag)
    230     {
    231         case stag_undef:      stack = U4Stack_Unclassified                              ; break ;  // 0 -> 0
    232         case stag_to_sci:     stack = U4Stack_ScintDiscreteReset                        ; break ;  // 1 -> 2
    233         case stag_to_bnd:     stack = U4Stack_BoundaryDiscreteReset                     ; break ;  // 2 -> 6 
    234         case stag_to_sca:     stack = U4Stack_RayleighDiscreteReset                     ; break ;  // 3 -> 4 




DONE : try artificially consuming 4 in A after every BR to see if it can kick back into line 
-----------------------------------------------------------------------------------------------

::

    epsilon:opticks blyth$ git add . 
    epsilon:opticks blyth$ git commit -m "try artificially consuming 4 in A after every BR to see if it can kick back into line "
    [master 4f1ca23a2] try artificially consuming 4 in A after every BR to see if it can kick back into line
     5 files changed, 386 insertions(+), 27 deletions(-)



qsim.h tail of propagate_to_boundary::

     890 
     891     flag = reflect ? BOUNDARY_REFLECT : BOUNDARY_TRANSMIT ;
     892 
     893 
     894 #ifdef DEBUG_TAG
     895     if( flag ==  BOUNDARY_REFLECT )
     896     {
     897         const float u_br_align_0 = curand_uniform(&rng) ;
     898         const float u_br_align_1 = curand_uniform(&rng) ;
     899         const float u_br_align_2 = curand_uniform(&rng) ;
     900         const float u_br_align_3 = curand_uniform(&rng) ;
     901 
     902         tagr.add( stag_to_sci    , u_br_align_0 );  // switch to stag_to_sci so stag.StepSplit will split it 
     903         tagr.add( stag_br_align_1, u_br_align_1 );
     904         tagr.add( stag_br_align_2, u_br_align_2 );
     905         tagr.add( stag_br_align_3, u_br_align_3 );
     906     }
     907 #endif
     908 
     909     return CONTINUE ;
     910 }
     911 

**after**

After using stag_to_sci for the first burn after BR the internals match too::

    In [3]: np.where( a.seq[:,0] != b.seq[:,0] )
    Out[3]: (array([], dtype=int64),)

    In [4]: np.where( a.flat != b.flat )
    Out[4]: (array([], dtype=int64), array([], dtype=int64))

    In [5]: np.where( an != bn )
    Out[5]: (array([], dtype=int64),)

    In [9]: np.where(afs != bfs )
    Out[9]: (array([], dtype=int64), array([], dtype=int64), array([], dtype=int64))


**before**

Succeeds to match histories of the 100, but the splitting of tags and flat 
is not matching, due to using stag_br_align_0 rather than stag_to_sci.::

    u4t
    ./U4RecorderTest.sh ab 

    In [3]: np.where( a.seq[:,0] != b.seq[:,0] )
    Out[3]: (array([], dtype=int64),)

    In [7]: np.where( a.flat != b.flat )
    Out[7]: (array([], dtype=int64), array([], dtype=int64))

    In [12]: np.where( an != bn )
    Out[12]: (array([ 3, 15, 21, 25, 36, 53, 54, 64]),)

    In [14]: an[an != bn],bn[an != bn]
    Out[14]: 
    (array([2, 2, 2, 2, 4, 4, 5, 4], dtype=uint8),
     array([3, 3, 3, 3, 5, 5, 7, 5], dtype=uint8))

    In [15]: afs[3]
    Out[15]: 
    array([[0.969, 0.495, 0.673, 0.563, 0.12 , 0.976, 0.136, 0.589, 0.491, 0.328],
           [0.911, 0.191, 0.964, 0.898, 0.624, 0.71 , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ]], dtype=float32)

    In [16]: bfs[3]
    Out[16]: 
    array([[0.969, 0.495, 0.673, 0.563, 0.12 , 0.976, 0.   , 0.   , 0.   , 0.   ],
           [0.136, 0.589, 0.491, 0.328, 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.911, 0.191, 0.964, 0.898, 0.624, 0.71 , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ]], dtype=float32)

    ## HMM : as 22 not 1 : it doesnt get folded

    In [17]: ats[3]
    Out[17]: 
    array([[ 1,  2,  3,  4,  5,  6, 22, 23, 24, 25],
           [ 1,  2,  3,  4,  7,  8,  0,  0,  0,  0],
           [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0]], dtype=uint8)

    In [18]: bts[3]
    Out[18]: 
    array([[2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 0, 0, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 9, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8)


    In [19]: afs[54]
    Out[19]: 
    array([[0.708, 0.08 , 0.197, 0.401, 0.378, 0.744, 0.   , 0.   , 0.   , 0.   ],
           [0.035, 0.371, 0.329, 0.114, 0.224, 0.987, 0.673, 0.133, 0.965, 0.555],
           [0.654, 0.516, 0.715, 0.407, 0.549, 0.993, 0.355, 0.348, 0.821, 0.422],
           [0.569, 0.602, 0.088, 0.955, 0.828, 0.806, 0.   , 0.   , 0.   , 0.   ],
           [0.245, 0.504, 0.179, 0.8  , 0.333, 0.717, 0.   , 0.   , 0.   , 0.   ]], dtype=float32)

    In [20]: bfs[54]
    Out[20]: 
    array([[0.708, 0.08 , 0.197, 0.401, 0.378, 0.744, 0.   , 0.   , 0.   , 0.   ],
           [0.035, 0.371, 0.329, 0.114, 0.224, 0.987, 0.   , 0.   , 0.   , 0.   ],
           [0.673, 0.133, 0.965, 0.555, 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.654, 0.516, 0.715, 0.407, 0.549, 0.993, 0.   , 0.   , 0.   , 0.   ],
           [0.355, 0.348, 0.821, 0.422, 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.569, 0.602, 0.088, 0.955, 0.828, 0.806, 0.   , 0.   , 0.   , 0.   ],
           [0.245, 0.504, 0.179, 0.8  , 0.333, 0.717, 0.   , 0.   , 0.   , 0.   ]], dtype=float32)

    In [21]: ats[54]
    Out[21]: 
    array([[ 1,  2,  3,  4,  5,  6,  0,  0,  0,  0],
           [ 1,  2,  3,  4,  5,  6, 22, 23, 24, 25],
           [ 1,  2,  3,  4,  5,  6, 22, 23, 24, 25],
           [ 1,  2,  3,  4,  5,  6,  0,  0,  0,  0],
           [ 1,  2,  3,  4,  7,  8,  0,  0,  0,  0]], dtype=uint8)

    In [23]: print(tag.label(ats[54,1]))
     0 :  1 :     to_sci : qsim::propagate_to_boundary u_to_sci burn  
     1 :  2 :     to_bnd : qsim::propagate_to_boundary u_to_bnd burn  
     2 :  3 :     to_sca : qsim::propagate_to_boundary u_scattering  
     3 :  4 :     to_abs : qsim::propagate_to_boundary u_absorption  
     4 :  5 :    at_burn : boundary burn  
     5 :  6 :     at_ref : u_reflect > TransCoeff  
     6 : 22 : br_align_0 : qsim::propagate_at_boundary tail u_br_align_0    
     7 : 23 : br_align_1 : qsim::propagate_at_boundary tail u_br_align_1    
     8 : 24 : br_align_2 : qsim::propagate_at_boundary tail u_br_align_2    
     9 : 25 : br_align_3 : qsim::propagate_at_boundary tail u_br_align_3    


    In [22]: bts[54]
    Out[22]: 
    array([[2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 0, 0, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 0, 0, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 9, 0, 0, 0, 0]], dtype=uint8)






DONE : check max_starts difference : tis caused by the B:StepTooSmall handling  
---------------------------------------------------------------------------------

stag.py::

     42     @classmethod
     43     def NumStarts(cls, tg):
     44         ns = np.zeros( (len(tg)), dtype=np.uint8 )
     45         for i in range(len(tg)):
     46             starts = np.where( tg[i] == tg[0,0] )[0]
     47             ns[i] = len(starts)
     48         pass
     49         return ns

    In [1]: an
    Out[1]: 
    array([3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 2, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 3, 3, 3, 3, 3, 3, 3, 3, 3,
           4, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3], dtype=uint8)

    In [2]: bn
    Out[2]: 
    array([3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 5, 7, 3, 3, 3, 3, 3, 3, 3, 3, 3,
           5, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3], dtype=uint8)

    In [3]: np.where( an != bn )
    Out[3]: (array([ 3, 15, 21, 25, 36, 53, 54, 64]),)


    In [7]: an[an != bn]
    Out[7]: array([2, 2, 2, 2, 4, 4, 4, 4], dtype=uint8)

    In [8]: bn[an != bn]
    Out[8]: array([3, 3, 3, 3, 5, 5, 7, 5], dtype=uint8)

    ## NORMALLY ONE EXTRA LINE, BAD APPLE 54 WITH 3 EXTRA LINES 

    In [4]: w8 = np.where( an != bn )[0]

    In [5]: seqhis_(a.seq[w8,0])
    Out[5]: 
    ['TO BR SA',
     'TO BR SA',
     'TO BR SA',
     'TO BR SA',
     'TO BT BR BT SA',
     'TO BT BR BT SA',
     'TO BT BR BT SA',
     'TO BT BR BT SA']

    In [6]: seqhis_(b.seq[w8,0])
    Out[6]: 
    ['TO BR SA',
     'TO BR SA',
     'TO BR SA',
     'TO BR SA',
     'TO BT BR BT SA',
     'TO BT BR BT SA',
     'TO BT BR BR BT SA',
     'TO BT BR BT SA']




DONE : investigate misaligned idx 54, check flat alignment : some alignment may be by chance
----------------------------------------------------------------------------------------------

::

    In [15]: seqhis_(a.seq[54,0])
    Out[15]: 'TO BT BR BT SA'

    In [16]: seqhis_(b.seq[54,0])
    Out[16]: 'TO BT BR BR BT SA'


    In [13]: ats[54]
    Out[13]: 
    array([[1, 2, 3, 4, 5, 6, 0, 0, 0, 0],
           [1, 2, 3, 4, 5, 6, 0, 0, 0, 0],
           [1, 2, 3, 4, 5, 6, 0, 0, 0, 0],
           [1, 2, 3, 4, 7, 8, 0, 0, 0, 0]], dtype=uint8)

    In [14]: bts[54]
    Out[14]: 
    array([[2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 0, 0, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 0, 0, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 9, 0, 0, 0, 0]], dtype=uint8)

    In [4]: afs[54]
    Out[4]: 
    array([[0.708, 0.08 , 0.197, 0.401, 0.378, 0.744, 0.   , 0.   , 0.   , 0.   ],
           [0.035, 0.371, 0.329, 0.114, 0.224, 0.987, 0.   , 0.   , 0.   , 0.   ],
           [0.673, 0.133, 0.965, 0.555, 0.654, 0.516, 0.   , 0.   , 0.   , 0.   ],
           [0.715, 0.407, 0.549, 0.993, 0.355, 0.348, 0.   , 0.   , 0.   , 0.   ]], dtype=float32)

    In [5]: bfs[54]
    Out[5]: 
    array([[0.708, 0.08 , 0.197, 0.401, 0.378, 0.744, 0.   , 0.   , 0.   , 0.   ],
           [0.035, 0.371, 0.329, 0.114, 0.224, 0.987, 0.   , 0.   , 0.   , 0.   ],
           [0.673, 0.133, 0.965, 0.555, 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.654, 0.516, 0.715, 0.407, 0.549, 0.993, 0.   , 0.   , 0.   , 0.   ],
           [0.355, 0.348, 0.821, 0.422, 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.569, 0.602, 0.088, 0.955, 0.828, 0.806, 0.   , 0.   , 0.   , 0.   ],
           [0.245, 0.504, 0.179, 0.8  , 0.333, 0.717, 0.   , 0.   , 0.   , 0.   ]], dtype=float32)





DONE : check a BR that does not show up as discrepant : thats just by chance
--------------------------------------------------------------------------------

Below shows that not appearing as discrepant for this BR (and presumably all BR) 
is by chance only as the flats are out of step due to B:StepTooSmall consuming 4 
with no corresponding consumption from A 

::

    In [7]: seqhis_(a.seq[:6,0])
    Out[7]: 
    ['TO BT BT SA',
     'TO BT BT SA',
     'TO BT BT SA',
     'TO BR SA',
     'TO BT BT SA',
     'TO BT BT SA']

    In [8]: seqhis_(b.seq[:6,0])
    Out[8]: 
    ['TO BT BT SA',
     'TO BT BT SA',
     'TO BT BT SA',
     'TO BR SA',      # 3
     'TO BT BT SA',
     'TO BT BT SA']

    In [13]: ats[3], afs[3]
    Out[13]: 
    (array([[1, 2, 3, 4, 5, 6, 0, 0, 0, 0],
            [1, 2, 3, 4, 7, 8, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8),
     array([[0.969, 0.495, 0.673, 0.563, 0.12 , 0.976, 0.   , 0.   , 0.   , 0.   ],
            [0.136, 0.589, 0.491, 0.328, 0.911, 0.191, 0.   , 0.   , 0.   , 0.   ],
            [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
            [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ]], dtype=float32))

    In [14]: bts[3], bfs[3]
    Out[14]: 
    (array([[2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
            [2, 6, 4, 3, 0, 0, 0, 0, 0, 0],
            [2, 6, 4, 3, 8, 9, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
            [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8),
     array([[0.969, 0.495, 0.673, 0.563, 0.12 , 0.976, 0.   , 0.   , 0.   , 0.   ],
            [0.136, 0.589, 0.491, 0.328, 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
            [0.911, 0.191, 0.964, 0.898, 0.624, 0.71 , 0.   , 0.   , 0.   , 0.   ],
            [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
            [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
            [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
            [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ]], dtype=float32))



DONE : checking flat consumption per step in stag.StepSplit
---------------------------------------------------------------

::

    In [1]: bfs.shape                                                                                                                                               
    Out[1]: (100, 7, 10)

    In [2]: bfs[0]    
    ## suspect all the extra zeros in B are coming from the StepTooSmall BR 
    ## from max_starts inconsistency ?
    Out[2]: 
    array([[0.74 , 0.438, 0.517, 0.157, 0.071, 0.463, 0.   , 0.   , 0.   , 0.   ],
           [0.228, 0.329, 0.144, 0.188, 0.915, 0.54 , 0.   , 0.   , 0.   , 0.   ],
           [0.975, 0.547, 0.653, 0.23 , 0.339, 0.761, 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ]], dtype=float32)

    In [3]: afs[0]
    Out[3]: 
    array([[0.74 , 0.438, 0.517, 0.157, 0.071, 0.463, 0.   , 0.   , 0.   , 0.   ],
           [0.228, 0.329, 0.144, 0.188, 0.915, 0.54 , 0.   , 0.   , 0.   , 0.   ],
           [0.975, 0.547, 0.653, 0.23 , 0.339, 0.761, 0.   , 0.   , 0.   , 0.   ],
           [0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ]], dtype=float32)

    In [4]:                                                                      



DONE : First Try for alignment : gives seqhis match for 99/100
-------------------------------------------------------------------

**after : seqhis aligns for 99/100**

::

    epsilon:opticks blyth$ git commit -m "reorganize stag.h enum with additions for preamble consumption alignment, use from qsim.h when DEBUG_TAG active"  
    [master b81a3f85b] reorganize stag.h enum with additions for preamble consumption alignment, use from qsim.h when DEBUG_TAG active
     6 files changed, 221 insertions(+), 99 deletions(-)
    epsilon:opticks blyth$ git push 
    Counting objects: 14, done.


    In [12]: np.where( a.seq[:,0] != b.seq[:,0] )
    Out[12]: (array([54]),)


    In [3]: ats[0]
    Out[3]: 
    array([[1, 2, 3, 4, 5, 6, 0, 0, 0, 0],
           [1, 2, 3, 4, 5, 6, 0, 0, 0, 0],
           [1, 2, 3, 4, 7, 8, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8)

    In [4]: bts[0]    ## huh what all the zeros ?
    Out[4]: 
    array([[2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 9, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8)

    In [6]: print(tag.label(at[0,:20]))
     0 :  1 :     to_sci : qsim::propagate_to_boundary u_to_sci burn  
     1 :  2 :     to_bnd : qsim::propagate_to_boundary u_to_bnd burn  
     2 :  3 :     to_sca : qsim::propagate_to_boundary u_scattering  
     3 :  4 :     to_abs : qsim::propagate_to_boundary u_absorption  
     4 :  5 :    at_burn : boundary burn  
     5 :  6 :     at_ref : u_reflect > TransCoeff  

     6 :  1 :     to_sci : qsim::propagate_to_boundary u_to_sci burn  
     7 :  2 :     to_bnd : qsim::propagate_to_boundary u_to_bnd burn  
     8 :  3 :     to_sca : qsim::propagate_to_boundary u_scattering  
     9 :  4 :     to_abs : qsim::propagate_to_boundary u_absorption  
    10 :  5 :    at_burn : boundary burn  
    11 :  6 :     at_ref : u_reflect > TransCoeff  

    12 :  1 :     to_sci : qsim::propagate_to_boundary u_to_sci burn  
    13 :  2 :     to_bnd : qsim::propagate_to_boundary u_to_bnd burn  
    14 :  3 :     to_sca : qsim::propagate_to_boundary u_scattering  
    15 :  4 :     to_abs : qsim::propagate_to_boundary u_absorption  
    16 :  7 :      sf_sd : qsim::propagate_at_surface ab/sd  
    17 :  8 :    sf_burn : qsim::propagate_at_surface burn  
    18 :  0 :      undef : undef  
    19 :  0 :      undef : undef  


    In [7]: print(stack.label(bt[0,:20]))
     0 :  2 : ScintDiscreteReset :   
     1 :  6 : BoundaryDiscreteReset :   
     2 :  4 : RayleighDiscreteReset :   
     3 :  3 : AbsorptionDiscreteReset :   
     4 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
     5 :  7 : BoundaryDiDiTransCoeff :   

     6 :  2 : ScintDiscreteReset :   
     7 :  6 : BoundaryDiscreteReset :   
     8 :  4 : RayleighDiscreteReset :   
     9 :  3 : AbsorptionDiscreteReset :   
    10 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
    11 :  7 : BoundaryDiDiTransCoeff :   

    12 :  2 : ScintDiscreteReset :   
    13 :  6 : BoundaryDiscreteReset :   
    14 :  4 : RayleighDiscreteReset :   
    15 :  3 : AbsorptionDiscreteReset :   
    16 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
    17 :  9 : AbsorptionEffDetect :   
    18 :  0 : Unclassified :   
    19 :  0 : Unclassified :   



**before : chance seqhis alignment only**

::

    In [8]: seqhis_(a.seq[0,0])
    Out[8]: 'TO BT BT SA'

    In [9]: seqhis_(b.seq[0,0])
    Out[9]: 'TO BT BT SA'

    In [11]: ats[0]
    Out[11]: 
    array([[ 1,  2,  9, 10,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  0,  0,  0,  0,  0,  0],
           [ 1,  2, 11, 12,  0,  0,  0,  0,  0,  0],
           [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0]], dtype=uint8)

    In [12]: bts[0]
    Out[12]: 
    array([[2, 6, {4, 3, 8, 7}, 0, 0, 0, 0],
           [2, 6, {4, 3, 8, 7}, 0, 0, 0, 0],
           [2, 6, {4, 3, 8, 9}, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8)

    In [13]: print(tag.label(at[0,:14]))
     0 :  1 :      to_sc : qsim::propagate_to_boundary u_scattering  
     1 :  2 :      to_ab : qsim::propagate_to_boundary u_absorption  
     2 :  9 :      at_bo : boundary burn  
     3 : 10 :      at_rf : u_reflect > TransCoeff  

     4 :  1 :      to_sc : qsim::propagate_to_boundary u_scattering  
     5 :  2 :      to_ab : qsim::propagate_to_boundary u_absorption  
     6 :  9 :      at_bo : boundary burn  
     7 : 10 :      at_rf : u_reflect > TransCoeff  

     8 :  1 :      to_sc : qsim::propagate_to_boundary u_scattering  
     9 :  2 :      to_ab : qsim::propagate_to_boundary u_absorption  
    10 : 11 :      sf_sd : qsim::propagate_at_surface ab/sd  
    11 : 12 :      sf_bu : qsim::propagate_at_surface burn  
    12 :  0 :      undef : undef  
    13 :  0 :      undef : undef  

    In [14]: print(stack.label(bt[0,:20]))
     0 :  2 : ScintDiscreteReset :   
     1 :  6 : BoundaryDiscreteReset :   
     2 :  4 : RayleighDiscreteReset :                        ## stack:4 equiv tag:1 
     3 :  3 : AbsorptionDiscreteReset :                      ## stack:3 equiv tag:2
     4 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :    ## stack:8 here equiv to tag:9 (also maps to tag:11) 
     5 :  7 : BoundaryDiDiTransCoeff :                       ## stack:7 equiv tag:10

     6 :  2 : ScintDiscreteReset :   
     7 :  6 : BoundaryDiscreteReset :   
     8 :  4 : RayleighDiscreteReset :   
     9 :  3 : AbsorptionDiscreteReset :   
    10 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
    11 :  7 : BoundaryDiDiTransCoeff :   

    12 :  2 : ScintDiscreteReset :   
    13 :  6 : BoundaryDiscreteReset :   
    14 :  4 : RayleighDiscreteReset :   
    15 :  3 : AbsorptionDiscreteReset :   
    16 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   ## stack:8 here maps to tag:11  (it also maps to tag:9)
    17 :  9 : AbsorptionEffDetect :                         ## stack:9 maps to tag:12  
    18 :  0 : Unclassified :   
    19 :  0 : Unclassified :   





DONE : adjust how StepTooSmall is handled to avoid messing up the consumption regularity 
---------------------------------------------------------------------------------------------

* HMM in CFG4 I recall doing some jump backs to stay aligned. Was that for StepTooSmall ?
* better to avoid such complications : better to add burns on other side
* goal is a *regular* easy to follow pattern of consumption that can be aligned with 

**setup**

::

    u4t
    ./U4RecorderTest_ab.sh 


**after : change to always call U4Process::ClearNumberOfInteractionLengthLeft even when StepTooSmall/NAN_ABORT**

::

    182 void U4Recorder::UserSteppingAction_Optical(const G4Step* step)
    183 {   
    ...
    197 
    198     bool first_point = current_photon.flagmask_count() == 1 ;  // first_point when single bit in the flag from genflag set in beginPhoton
    199     if(first_point)
    200     {
    201         U4StepPoint::Update(current_photon, pre);
    202         sev->pointPhoton(label);  // saves SEvt::current_photon/rec/record/prd into sevent 
    203     }
    204 
    205     unsigned flag = U4StepPoint::Flag(post) ;
    206     if( flag == 0 ) LOG(error) << " ERR flag zero : post " << U4StepPoint::Desc(post) ;
    207     assert( flag > 0 );
    208 
    209     if( flag == NAN_ABORT )
    210     {
    211         LOG(error) << " skip post saving for StepTooSmall label.id " << label.id  ;
    212     }
    213     else
    214     {
    215         G4TrackStatus tstat = track->GetTrackStatus();
    216         Check_TrackStatus_Flag(tstat, flag);
    217 
    218         U4StepPoint::Update(current_photon, post);
    219         current_photon.set_flag( flag );
    220         sev->pointPhoton(label);         // save SEvt::current_photon/rec/seq/prd into sevent 
    221     }
    222     U4Process::ClearNumberOfInteractionLengthLeft(*track, *step);
    223 }


::

    In [4]: bts.shape
    Out[4]: (100, 7, 10)

    In [5]: bts[0]
    Out[5]: 
    array([[2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 9, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8)

::

    In [10]: np.all(np.logical_or(bts[:,:,0] == 2, bts[:,:,0] == 0))
    Out[10]: True

    In [11]: np.all(np.logical_or(bts[:,:,1] == 6, bts[:,:,1] == 0))
    Out[11]: True

    In [12]: np.all(np.logical_or(bts[:,:,2] == 4, bts[:,:,2] == 0))
    Out[12]: True

    In [13]: np.all(np.logical_or(bts[:,:,3] == 3, bts[:,:,3] == 0))
    Out[13]: True

    In [14]: np.all(np.logical_or(bts[:,:,4] == 8, bts[:,:,4] == 0))
    Out[14]: True

    ## SO WHEN NOT ZERO : ALL STEPS START THE SAME : (2,6,4,3,8) 

    In [16]: print(stack.label(bt[0,:20]))
     0 :  2 : ScintDiscreteReset :   
     1 :  6 : BoundaryDiscreteReset :   
     2 :  4 : RayleighDiscreteReset :   
     3 :  3 : AbsorptionDiscreteReset :   
     4 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
     5 :  7 : BoundaryDiDiTransCoeff :   

     6 :  2 : ScintDiscreteReset :   
     7 :  6 : BoundaryDiscreteReset :   
     8 :  4 : RayleighDiscreteReset :   
     9 :  3 : AbsorptionDiscreteReset :   
    10 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
    11 :  7 : BoundaryDiDiTransCoeff :   

    12 :  2 : ScintDiscreteReset :   
    13 :  6 : BoundaryDiscreteReset :   
    14 :  4 : RayleighDiscreteReset :   
    15 :  3 : AbsorptionDiscreteReset :   
    16 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
    17 :  9 : AbsorptionEffDetect :   
    18 :  0 : Unclassified :   
    19 :  0 : Unclassified :   




**before**

::

    In [1]: w8 = np.where(bts[:,:,2] == 8 )
    In [2]: w8
    Out[2]: (array([ 3, 15, 21, 25, 36, 53, 54, 64]), array([2, 2, 2, 2, 3, 3, 3, 3]))

    In [3]: w8 = np.where(bts[:,:,2] == 8 )[0]

    In [5]: b.seq[w8,0]
    Out[5]: array([  2237,   2237,   2237,   2237, 576461, 576461, 576461, 576461], dtype=uint64)

    In [6]: seqhis_(b.seq[w8,0])
    Out[6]: 
    ['TO BR SA',
     'TO BR SA',
     'TO BR SA',
     'TO BR SA',
     'TO BT BR BT SA',
     'TO BT BR BT SA',
     'TO BT BR BT SA',
     'TO BT BR BT SA']

    In [15]: sh = seqhis_(b.seq[:,0])
    In [17]: for i in range(len(sh)): 
        ...:     if sh[i].find("BR")>-1: print(i) 
        ...:
    3
    15
    21
    25
    36
    53
    54
    64

All 8 BR in 100 have same problem, seems to be the step after the BR that has messed up consumption




DONE : folding A:tags and B:stacks arrays for clarity and easier querying using stag.StepSplit 
---------------------------------------------------------------------------------------------------
::

    In [3]: seqhis_(a.seq[:5,0])
    Out[3]: ['TO BT BT SA', 'TO BT BT SA', 'TO BT BT SA', 'TO BT BT SA', 'TO BT BT SA']

    In [4]: seqhis_(b.seq[:5,0])
    Out[4]: ['TO BT BT SA', 'TO BT BT SA', 'TO BT BT SA', 'TO BR SA', 'TO BT BT SA']


Consumption pattern expected to always have same start to each steppoint from the stack Reset deciding
on what process will win the step.  So rearranging array into those steps makes it easier to follow and query::

    In [8]: at[:5,:20]   # A:tags
    Out[8]: 
    array([[ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0,  0,  0,  0,  0]], dtype=uint8)

    In [9]: bt[:5,:20]   # B:stacks
    Out[9]: 
    array([[2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 9, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 9, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 9, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 2, 6, 8, 9, 0, 0, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 9, 0, 0]], dtype=uint8)

::

    In [10]: at[0]
    Out[10]: array([ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0], dtype=uint8)

::

    In [18]: starts = np.where( at[0] == 1 )[0] ; starts
    Out[18]: array([0, 4, 8])

    ends = np.where( at[0] == 0 )   
    end = ends[0][0] 

    In [21]: at[0,0:4]
    Out[21]: array([ 1,  2,  9, 10], dtype=uint8)

    In [22]: at[0,4:8]
    Out[22]: array([ 1,  2,  9, 10], dtype=uint8)

    In [56]: at[0,8:end]
    Out[56]: array([ 1,  2, 11, 12], dtype=uint8)

    ats = np.zeros( (5, 10), dtype=np.uint8 ) 
    ats[0,0:4] = at[0,0:4]  
    ats[1,0:4] = at[0,4:8]  
    ats[2,0:4] = at[0,8:end]   


stag.py::

     41     @classmethod
     42     def StepSplit(cls, tg, step_slot=10):
     43         """
     44         :param tg: unpacked tag array of shape (n, SLOTS)
     45         :param step_slot: max random throws per step  
     46         :param tgs: step split tag array of shape (n, max_step, step_slot) 
     47 
     48         In [4]: at[0]
     49         Out[4]: array([ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0], dtype=uint8)
     50 
     51         In [8]: ats[0]
     52         Out[8]: 
     53         array([[ 1,  2,  9, 10,  0,  0,  0,  0,  0,  0],
     54                [ 1,  2,  9, 10,  0,  0,  0,  0,  0,  0],
     55                [ 1,  2, 11, 12,  0,  0,  0,  0,  0,  0],
     56                [ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0]], dtype=uint8)
     57 
     58         """
     59 
     60         max_starts = 0
     61         for i in range(len(tg)):
     62             starts = np.where( tg[i] == tg[0,0] )[0]
     63             if len(starts) > max_starts: max_starts = len(starts)
     64         pass
     65         
     66         tgs = np.zeros((len(tg), max_starts, step_slot), dtype=np.uint8)
     67         for i in range(len(tg)): 
     68             starts = np.where( tg[i] == tg[0,0] )[0]
     69             ends = np.where( tg[i] == 0 )[0] 
     70             end = ends[0] if len(ends) > 0 else len(tg[i])   ## handle when dont get zero due to truncation
     71             for j in range(len(starts)):
     72                 st = starts[j]
     73                 en = starts[j+1] if j+1 < len(starts) else end
     74                 tgs[i, j,0:en-st] = tg[i,st:en] 
     75             pass
     76         pass
     77         return tgs



Difficult to interpret whats happening when have truncation::

    In [2]: ats[53]
    Out[2]: 
    array([[ 1,  2,  9, 10,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  0,  0,  0,  0,  0,  0],
           [ 1,  2, 11, 12,  0,  0,  0,  0,  0,  0]], dtype=uint8)

    In [3]: bts[53]
    Out[3]: 
    array([[2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 0, 0, 0, 0, 0, 0],
           [2, 6, 8, 7, 0, 0, 0, 0, 0, 0],
           [2, 6, 4, 3, 0, 0, 0, 0, 0, 0]], dtype=uint8)

    In [4]: seqhis_(a.seq[53,0])
    Out[4]: 'TO BT BR BR BR BT SA'

    In [5]: seqhis_(b.seq[53,0])
    Out[5]: 'TO BT BR BT SA'

    In [6]: at[53]
    Out[6]: array([ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2,  9, 10,  1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12], dtype=uint8)

    In [7]: bt[53]
    Out[7]: array([2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 2, 6, 8, 7, 2, 6, 4, 3], dtype=uint8)


    In [1]: print(stack.label(bt[53]))
     0 :  2 : ScintDiscreteReset :   
     1 :  6 : BoundaryDiscreteReset :   
     2 :  4 : RayleighDiscreteReset :   
     3 :  3 : AbsorptionDiscreteReset :   
     4 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
     5 :  7 : BoundaryDiDiTransCoeff :   

     6 :  2 : ScintDiscreteReset :   
     7 :  6 : BoundaryDiscreteReset :   
     8 :  4 : RayleighDiscreteReset :   
     9 :  3 : AbsorptionDiscreteReset :   
    10 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
    11 :  7 : BoundaryDiDiTransCoeff :   

    12 :  2 : ScintDiscreteReset :   
    13 :  6 : BoundaryDiscreteReset :   
    14 :  4 : RayleighDiscreteReset :   
    15 :  3 : AbsorptionDiscreteReset :   

    16 :  2 : ScintDiscreteReset :   
    17 :  6 : BoundaryDiscreteReset :   
    18 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
    19 :  7 : BoundaryDiDiTransCoeff :   
    ##  HMM: ONLY 2 RESET, NOT NORMAL GANG OF 4 ?

    20 :  2 : ScintDiscreteReset :   
    21 :  6 : BoundaryDiscreteReset :   
    22 :  4 : RayleighDiscreteReset :   
    23 :  3 : AbsorptionDiscreteReset :   

How often ? 8/100::

    In [9]: np.where(bts[:,:,2] == 8 )
    Out[9]: (array([ 3, 15, 21, 25, 36, 53, 54, 64]), array([2, 2, 2, 2, 3, 3, 3, 3]))

    In [10]: bts[3]
    Out[10]: 
    array([[2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 0, 0, 0, 0, 0, 0],
           [2, 6, 8, 9, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8)

    In [11]: bts[15]
    Out[11]: 
    array([[2, 6, 4, 3, 8, 7, 0, 0, 0, 0],
           [2, 6, 4, 3, 0, 0, 0, 0, 0, 0],
           [2, 6, 8, 9, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0]], dtype=uint8)


Whats special about those 8 ? All have StepTooSmall skip outs::

    2022-06-24 12:20:06.817 INFO  [30005984] [U4RecorderTest::GeneratePrimaries@119] ]
    2022-06-24 12:20:06.817 INFO  [30005984] [U4Recorder::BeginOfEventAction@52] 
    2022-06-24 12:20:07.123 ERROR [30005984] [U4StepPoint::Flag@123]  fGeomBoundary  U4OpBoundaryProcessStatus::Name StepTooSmall flag NAN_ABORT
    2022-06-24 12:20:07.124 ERROR [30005984] [U4Recorder::UserSteppingAction_Optical@209]  skipping StepTooSmall label.id 64
    2022-06-24 12:20:07.214 ERROR [30005984] [U4StepPoint::Flag@123]  fGeomBoundary  U4OpBoundaryProcessStatus::Name StepTooSmall flag NAN_ABORT
    2022-06-24 12:20:07.214 ERROR [30005984] [U4Recorder::UserSteppingAction_Optical@209]  skipping StepTooSmall label.id 54
    2022-06-24 12:20:07.227 ERROR [30005984] [U4StepPoint::Flag@123]  fGeomBoundary  U4OpBoundaryProcessStatus::Name StepTooSmall flag NAN_ABORT
    2022-06-24 12:20:07.227 ERROR [30005984] [U4Recorder::UserSteppingAction_Optical@209]  skipping StepTooSmall label.id 53
    2022-06-24 12:20:07.379 ERROR [30005984] [U4StepPoint::Flag@123]  fGeomBoundary  U4OpBoundaryProcessStatus::Name StepTooSmall flag NAN_ABORT
    2022-06-24 12:20:07.379 ERROR [30005984] [U4Recorder::UserSteppingAction_Optical@209]  skipping StepTooSmall label.id 36
    2022-06-24 12:20:07.476 ERROR [30005984] [U4StepPoint::Flag@123]  fGeomBoundary  U4OpBoundaryProcessStatus::Name StepTooSmall flag NAN_ABORT
    2022-06-24 12:20:07.476 ERROR [30005984] [U4Recorder::UserSteppingAction_Optical@209]  skipping StepTooSmall label.id 25
    2022-06-24 12:20:07.509 ERROR [30005984] [U4StepPoint::Flag@123]  fGeomBoundary  U4OpBoundaryProcessStatus::Name StepTooSmall flag NAN_ABORT
    2022-06-24 12:20:07.509 ERROR [30005984] [U4Recorder::UserSteppingAction_Optical@209]  skipping StepTooSmall label.id 21
    2022-06-24 12:20:07.561 ERROR [30005984] [U4StepPoint::Flag@123]  fGeomBoundary  U4OpBoundaryProcessStatus::Name StepTooSmall flag NAN_ABORT
    2022-06-24 12:20:07.561 ERROR [30005984] [U4Recorder::UserSteppingAction_Optical@209]  skipping StepTooSmall label.id 15
    2022-06-24 12:20:07.666 ERROR [30005984] [U4StepPoint::Flag@123]  fGeomBoundary  U4OpBoundaryProcessStatus::Name StepTooSmall flag NAN_ABORT
    2022-06-24 12:20:07.666 ERROR [30005984] [U4Recorder::UserSteppingAction_Optical@209]  skipping StepTooSmall label.id 3
    2022-06-24 12:20:07.693 INFO  [30005984] [U4Recorder::EndOfEventAction@53] 
    2022-06-24 12:20:07.693 INFO  [30005984] [U4Recorder::EndOfRunAction@51] 


Increase stag.h/stag.py:NSEQ to 4 increases SLOTS to 48, avoiding truncation::

    In [3]: print(stack.label(bt[53,:27]))
     0 :  2 : ScintDiscreteReset :   
     1 :  6 : BoundaryDiscreteReset :   
     2 :  4 : RayleighDiscreteReset :   
     3 :  3 : AbsorptionDiscreteReset :   
     4 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
     5 :  7 : BoundaryDiDiTransCoeff :   

     6 :  2 : ScintDiscreteReset :   
     7 :  6 : BoundaryDiscreteReset :   
     8 :  4 : RayleighDiscreteReset :   
     9 :  3 : AbsorptionDiscreteReset :   
    10 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
    11 :  7 : BoundaryDiDiTransCoeff :   

    12 :  2 : ScintDiscreteReset :   
    13 :  6 : BoundaryDiscreteReset :   
    14 :  4 : RayleighDiscreteReset :   
    15 :  3 : AbsorptionDiscreteReset :   

    16 :  2 : ScintDiscreteReset :   
    17 :  6 : BoundaryDiscreteReset :   
    18 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
    19 :  7 : BoundaryDiDiTransCoeff :   

    20 :  2 : ScintDiscreteReset :   
    21 :  6 : BoundaryDiscreteReset :   
    22 :  4 : RayleighDiscreteReset :   
    23 :  3 : AbsorptionDiscreteReset :   
    24 :  8 : BoundaryBurn_SurfaceReflectTransmitAbsorb :   
    25 :  9 : AbsorptionEffDetect :   
    26 :  0 : Unclassified :   



Unaligned initial small geometry
----------------------------------

::

    In [17]: seqhis_(a.seq[:6,0])
    Out[17]: 
    ['TO BT BT SA',
     'TO BT BT SA',
     'TO BT BT SA',
     'TO BT BT SA',
     'TO BT BT SA',
     'TO BR SA']

    In [18]: seqhis_(b.seq[:6,0])
    Out[18]: 
    ['TO BT BT SA',
     'TO BT BT SA',
     'TO BT BT SA',
     'TO BR SA',
     'TO BT BT SA',
     'TO BT BT SA']

    ## when the flat are there they match 

    In [15]: a.flat[:6,:14]
    Out[15]: 
    array([[0.74 , 0.438, 0.517, 0.157, 0.071, 0.463, 0.228, 0.329, 0.144, 0.188, 0.915, 0.54 , 0.   , 0.   ],
           [0.921, 0.46 , 0.333, 0.373, 0.49 , 0.567, 0.08 , 0.233, 0.509, 0.089, 0.007, 0.954, 0.   , 0.   ],
           [0.039, 0.25 , 0.184, 0.962, 0.521, 0.94 , 0.831, 0.41 , 0.082, 0.807, 0.695, 0.618, 0.   , 0.   ],
           [0.969, 0.495, 0.673, 0.563, 0.12 , 0.976, 0.136, 0.589, 0.491, 0.328, 0.911, 0.191, 0.   , 0.   ],
           [0.925, 0.053, 0.163, 0.89 , 0.567, 0.241, 0.494, 0.321, 0.079, 0.148, 0.599, 0.426, 0.   , 0.   ],
           [0.446, 0.338, 0.207, 0.985, 0.403, 0.178, 0.46 , 0.16 , 0.   , 0.   , 0.   , 0.   , 0.   , 0.   ]], dtype=float32)


    In [16]: b.flat[:6,:14]
    Out[16]: 
    array([[0.74 , 0.438, 0.517, 0.157, 0.071, 0.463, 0.228, 0.329, 0.144, 0.188, 0.915, 0.54 , 0.   , 0.   ],
           [0.921, 0.46 , 0.333, 0.373, 0.49 , 0.567, 0.08 , 0.233, 0.509, 0.089, 0.007, 0.954, 0.   , 0.   ],
           [0.039, 0.25 , 0.184, 0.962, 0.521, 0.94 , 0.831, 0.41 , 0.082, 0.807, 0.695, 0.618, 0.   , 0.   ],
           [0.969, 0.495, 0.673, 0.563, 0.12 , 0.976, 0.136, 0.589, 0.491, 0.328, 0.   , 0.   , 0.   , 0.   ],
           [0.925, 0.053, 0.163, 0.89 , 0.567, 0.241, 0.494, 0.321, 0.079, 0.148, 0.599, 0.426, 0.   , 0.   ],
           [0.446, 0.338, 0.207, 0.985, 0.403, 0.178, 0.46 , 0.16 , 0.361, 0.62 , 0.45 , 0.306, 0.   , 0.   ]], dtype=float32)


    In [13]: at[:6, :14]
    Out[13]: 
    array([[ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0],
           [ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0],
           [ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0],
           [ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0],
           [ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0],
           [ 1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0,  0,  0]], dtype=uint8)


    # A: step preamble deciding which process wins is 1,2 

    In [9]: print(tag.label(at[0,:14]))
     0 :  1 :      to_sc : qsim::propagate_to_boundary u_scattering 
     1 :  2 :      to_ab : qsim::propagate_to_boundary u_absorption 
     2 :  9 :      at_bo : boundary burn 
     3 : 10 :      at_rf : u_reflect > TransCoeff 
     4 :  1 :      to_sc : qsim::propagate_to_boundary u_scattering 
     5 :  2 :      to_ab : qsim::propagate_to_boundary u_absorption 
     6 :  9 :      at_bo : boundary burn 
     7 : 10 :      at_rf : u_reflect > TransCoeff 
     8 :  1 :      to_sc : qsim::propagate_to_boundary u_scattering 
     9 :  2 :      to_ab : qsim::propagate_to_boundary u_absorption 
    10 : 11 :      sf_sd : qsim::propagate_at_surface ab/sd 
    11 : 12 :      sf_bu : qsim::propagate_at_surface burn 
    12 :  0 :      undef : undef 
    13 :  0 :      undef : undef 

    In [10]: print(tag.label(at[5,:14]))
     0 :  1 :      to_sc : qsim::propagate_to_boundary u_scattering 
     1 :  2 :      to_ab : qsim::propagate_to_boundary u_absorption 
     2 :  9 :      at_bo : boundary burn 
     3 : 10 :      at_rf : u_reflect > TransCoeff 
     4 :  1 :      to_sc : qsim::propagate_to_boundary u_scattering 
     5 :  2 :      to_ab : qsim::propagate_to_boundary u_absorption 
     6 : 11 :      sf_sd : qsim::propagate_at_surface ab/sd 
     7 : 12 :      sf_bu : qsim::propagate_at_surface burn 
     8 :  0 :      undef : undef 
     9 :  0 :      undef : undef 
    10 :  0 :      undef : undef 
    11 :  0 :      undef : undef 
    12 :  0 :      undef : undef 
    13 :  0 :      undef : undef 

    In [14]: bt[:6, :14]
    Out[14]: 
    array([[2, 6, 4, 3, 8, 7, 2, 6, 8, 7, 2, 6, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 8, 7, 2, 6, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 8, 7, 2, 6, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 2, 6, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 8, 7, 2, 6, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 8, 7, 2, 6, 0, 0]], dtype=uint8)

    # step preamble deciding on winner process is 2,6,4,3 
    # BUT that does not fully re-run for each step getting only 2,6 for subsequent



    In [19]: print(stack.label(bt[0,:14]))
     0 :  2 : ScintDiscreteReset :  
     1 :  6 : BoundaryDiscreteReset :  
     2 :  4 : RayleighDiscreteReset :  
     3 :  3 : AbsorptionDiscreteReset :  

     4 :  8 : BoundaryBurn :  
     5 :  7 : BoundaryDiDi :  

     6 :  2 : ScintDiscreteReset :  
     7 :  6 : BoundaryDiscreteReset :  

     8 :  8 : BoundaryBurn :  
     9 :  7 : BoundaryDiDi :  

    10 :  2 : ScintDiscreteReset :  
    11 :  6 : BoundaryDiscreteReset :  
    12 :  0 : Unclassified :  
    13 :  0 : Unclassified :  


DONE : observe how consumption changes when use U4Process::ClearNumberOfInteractionLengthLeft 
--------------------------------------------------------------------------------------------------

* U4Process::ClearNumberOfInteractionLengthLeft called from tail of U4Recorder::UserSteppingAction_Optical

::

    182 void U4Recorder::UserSteppingAction_Optical(const G4Step* step)
    183 {
    ...
    258     if( tstat == fAlive )
    259     {
    260         U4Process::ClearNumberOfInteractionLengthLeft(*track, *step);
    261     }
    262 


* with this the step point preamble now 2,6,4,3 with all 4 process reset for every step point
* the advantage of this is its simplicity and similarity of each step point 

* the preamble consumption can loosely be regarded as the arrows between flag points, 
  that act to decide what the next history flag will be::

  TO->BT->BT->SA 

* where does SA fit into this ? B:G4 is getting NoRINDEX truncated ?
  but A actually finds perfectAbsorbSurface boundary

* DONE: added Geant4 surface equivalent on the Rock///Air boundary  
  which succeeds to avoid the dirty NoRINDEX truncation 


::

    In [6]: bt[:5,:20]
    Out[6]: 
    array([[2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 2, 6, 0, 0, 0, 0, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 0, 0, 0, 0]], dtype=uint8)

    In [2]: print(stack.label(bt[0,:20]))
     0 :  2 : ScintDiscreteReset :  
     1 :  6 : BoundaryDiscreteReset :  
     2 :  4 : RayleighDiscreteReset :  
     3 :  3 : AbsorptionDiscreteReset :  
     4 :  8 : BoundaryBurn :  
     5 :  7 : BoundaryDiDi :  

     6 :  2 : ScintDiscreteReset :  
     7 :  6 : BoundaryDiscreteReset :  
     8 :  4 : RayleighDiscreteReset :  
     9 :  3 : AbsorptionDiscreteReset :  
    10 :  8 : BoundaryBurn :  
    11 :  7 : BoundaryDiDi :  

    12 :  2 : ScintDiscreteReset :  
    13 :  6 : BoundaryDiscreteReset :  
    14 :  4 : RayleighDiscreteReset :  
    15 :  3 : AbsorptionDiscreteReset :  

    16 :  0 : Unclassified :  
    17 :  0 : Unclassified :  
    18 :  0 : Unclassified :  
    19 :  0 : Unclassified :  


    ## After remove the NoRINDEX kludge and add the G4OpticalSurface
    ## get additional tail of 8,9 

    In [2]: bt[:5,:20]
    Out[2]: 
    array([[2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 9, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 9, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 9, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 2, 6, 8, 9, 0, 0, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 9, 0, 0]], dtype=uint8)


    In [1]: print(stack.label(bt[0,:20]))
     0 :  2 : ScintDiscreteReset :  
     1 :  6 : BoundaryDiscreteReset :  
     2 :  4 : RayleighDiscreteReset :  
     3 :  3 : AbsorptionDiscreteReset :  
     4 :  8 : BoundaryReflectTransmitAbsorb :  
     5 :  7 : BoundaryDiDiTransCoeff : 

     6 :  2 : ScintDiscreteReset :  
     7 :  6 : BoundaryDiscreteReset :  
     8 :  4 : RayleighDiscreteReset :  
     9 :  3 : AbsorptionDiscreteReset :  
    10 :  8 : BoundaryReflectTransmitAbsorb :  
    11 :  7 : BoundaryDiDiTransCoeff :  

    12 :  2 : ScintDiscreteReset :  
    13 :  6 : BoundaryDiscreteReset :  
    14 :  4 : RayleighDiscreteReset :  
    15 :  3 : AbsorptionDiscreteReset :  
    16 :  8 : BoundaryReflectTransmitAbsorb :  
    17 :  9 : AbsorptionEffDetect :  

    18 :  0 : Unclassified :  
    19 :  0 : Unclassified :  


    In [4]: at[:5,:20]
    Out[4]: 
    array([[ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0,  0,  0,  0,  0],
           [ 1,  2,  9, 10,  1,  2,  9, 10,  1,  2, 11, 12,  0,  0,  0,  0,  0,  0,  0,  0]], dtype=uint8)


    TO->BT->BT->SA 

    In [5]: print(tag.label(at[0,:20]))
     0 :  1 :      to_sc : qsim::propagate_to_boundary u_scattering 
     1 :  2 :      to_ab : qsim::propagate_to_boundary u_absorption 
     2 :  9 :      at_bo : boundary burn 
     3 : 10 :      at_rf : u_reflect > TransCoeff 

     4 :  1 :      to_sc : qsim::propagate_to_boundary u_scattering 
     5 :  2 :      to_ab : qsim::propagate_to_boundary u_absorption 
     6 :  9 :      at_bo : boundary burn 
     7 : 10 :      at_rf : u_reflect > TransCoeff 

     8 :  1 :      to_sc : qsim::propagate_to_boundary u_scattering 
     9 :  2 :      to_ab : qsim::propagate_to_boundary u_absorption 

    10 : 11 :      sf_sd : qsim::propagate_at_surface ab/sd 
    11 : 12 :      sf_bu : qsim::propagate_at_surface burn 

    12 :  0 :      undef : undef 
    13 :  0 :      undef : undef 
    14 :  0 :      undef : undef 
    15 :  0 :      undef : undef 
    16 :  0 :      undef : undef 
    17 :  0 :      undef : undef 
    18 :  0 :      undef : undef 
    19 :  0 :      undef : undef 


* adding two burns at step front to A would bring them into line 
* at_surface difference at the end due to the NoRINDEX Rock trick probably ?

  * DONE : ADD A GEANT4 SURFACE TO THE TEST GEOMETRY TO MAKE THE TAIL POSSIBLE TO ALIGN WITH


Try with::

    182 void U4Recorder::UserSteppingAction_Optical(const G4Step* step)
    183 {
    ...
    258     //if( tstat == fAlive )
    259     {
    260         U4Process::ClearNumberOfInteractionLengthLeft(*track, *step);
    261     }
    262 
    263 
    264 }

Seems no difference, presumably all fAlive ?::

    In [1]: bt[:5,:20]
    Out[1]: 
    array([[2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 2, 6, 0, 0, 0, 0, 0, 0, 0, 0],
           [2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 8, 7, 2, 6, 4, 3, 0, 0, 0, 0]], dtype=uint8)






DONE : checked storch_test.sh MOCK_CURAND input photons match on laptop and workstation
------------------------------------------------------------------------------------------

Confirmed perfect match with input photons generated on Linux workstation and Apple laptop::

    cd ~/opticks/sysrap/tests
    ./storch_test.sh       # remote  
    ./storch_test.sh       # local  
    ./storch_test.sh grab  # local  
    ./storch_test.sh cf  # local using sysrap/tests/storch_test_cf.py    


