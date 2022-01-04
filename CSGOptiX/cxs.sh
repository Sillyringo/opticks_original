#!/bin/bash -l 

BASH_FOLDER="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

usage(){ cat << EOU
cxs.sh : hybrid rendering/simulation machinery, eg creating 2D ray trace cross sections
========================================================================================

TODO: fix the running directory to avoid dropping files in repo


::

    ISEL=0,1,3,4,5 ./cxs.sh ana       # select which boundaries to include in plot 

    XX=-208,208 ZZ=-15.2,15.2 ./cxs.sh 

Two envvars MOI and CEGS configure the gensteps.

The MOI string has form meshName:meshOrdinal:instanceIdx 
and is used to lookup the center extent from the CSGFoundry 
geometry. Examples::

    MOI=Hama
    MOI=Hama:0:0   
    CEGS=16:0:9:200                 # nx:ny:nz:num_photons
    CEGS=16:0:9:200:17700:0:0:200   # nx:ny:nz:num_photons:cx:cy:cz:ew

The CEGS envvar configures an *(nx,ny,nz)* grid from -nx->nx -ny->ny -nz->nz
of integers which are used to mutiply the extent from the MOI center-extent.
The *num_photons* is the number of photons for each of the grid gensteps.

* as the gensteps are currently xz-planar it makes sense to use *ny=0*
* to get a non-distorted jpg the nx:nz should follow the aspect ratio of the frame 

::

    In [1]: sz = np.array( [1920,1080] )
    In [5]: 9*sz/1080
    Out[5]: array([16.,  9.])

Instead of using the center-extent of the MOI selected solid, it is 
possible to directly enter the center-extent in integer mm for 
example adding "17700:0:0:200"

As the extent determines the spacing of the grid of gensteps, it is 
good to set a value of slightly less than the extent of the smallest
piece of geometry to try to get a genstep to land inside. 
Otherwise inner layers can be missed. 

EOU
}

msg="=== $BASH_SOURCE : "

#geom=Hama_1
#geom=HamaXZ_1
#geom=HamaYZ_1
#geom=HamaXY_1

#geom=Hama_2
#geom=Hama_4
#geom=Hama_8
#geom=Hama_16

#geom=uni_acrylic3_0
#geom=uni_acrylic1_0

#geom=XJfixtureConstructionXZ_0
#geom=XJfixtureConstructionYZ_0

#geom=XJfixtureConstructionXZ_1
#geom=XJfixtureConstructionYZ_1

#geom=XJfixtureConstructionTP_1
#geom=XJfixtureConstructionRT_1
#geom=XJfixtureConstructionRP_1
#geom=XJfixtureConstructionRP_55

geom=XJfixtureConstructionTR_55
#geom=XJfixtureConstructionPR_55

export GEOM=${GEOM:-$geom}

isel=
cfbase=
ce_offset=0
ce_scale=0
gsplot=1

if [ "$GEOM" == "Hama_1" ]; then

    moi=Hama
    cegs=16:0:9:500   # XZ works 
    gridscale=0.10

elif [ "$GEOM" == "HamaXZ_1" ]; then

    moi=Hama
    cegs=16:0:9:500   
    gridscale=0.10

elif [ "$GEOM" == "HamaYZ_1" ]; then

    moi=Hama
    cegs=0:16:9:500  
    gridscale=0.10

elif [ "$GEOM" == "HamaXY_1" ]; then

    moi=Hama
    cegs=16:9:0:500 
    gridscale=0.10

elif [ "$GEOM" == "Hama_2" ]; then

    moi=Hama
    cegs=32:0:18:500
    gridscale=0.10

elif [ "$GEOM" == "Hama_4" ]; then

    moi=Hama
    cegs=64:0:36:100
    #gridscale=0.10
    gridscale=0.20
    gsplot=0

elif [ "$GEOM" == "Hama_8" ]; then

    moi=Hama
    cegs=128:0:72:100
    gridscale=0.40
    gsplot=0

elif [ "$GEOM" == "Hama_16" ]; then

    ##  CUDA error on synchronize with error 'an illegal memory access was encountered' (/data/blyth/junotop/opticks/CSGOptiX/CSGOptiX.cc:342)
    moi=Hama
    cegs=256:0:144:10
    gridscale=0.20
    gsplot=0


elif [ "$GEOM" == "uni_acrylic1_0" ]; then
    moi=uni_acrylic1
    cegs=16:0:9:100
    gridscale=0.05
elif [ "$GEOM" == "uni_acrylic3_0" ]; then
    ## when use the option --additionacrylic-simplify-csg the uni_acrylic3 is not present 
    ## instead get uni_acrylic1 : is that OK? 
    moi=uni_acrylic3
    cegs=16:0:9:100
    #cegs=0:0:0:1000
    #cegs=16:4:9:100
    gridscale=0.05
elif [ "$GEOM" == "uni_acrylic3_wide" ]; then
    moi=uni_acrylic3
    cegs=32:0:18:100
    gridscale=0.025
elif [ "$GEOM" == "uni_acrylic3_tight" ]; then
    note="very tight grid to get into close corners"
    moi=uni_acrylic3
    cegs=16:0:9:100
    gridscale=0.025

elif [ "$GEOM" == "XJfixtureConstructionXZ_0" ]; then

    moi="solidXJfixture:10"
    cegs=16:0:9:100           
    gridscale=0.07

    ce_offset=1    # pre-tangential-frame approach  
    ce_scale=1 

elif [ "$GEOM" == "XJfixtureConstructionYZ_0" ]; then

    moi="solidXJfixture:10"
    cegs=0:16:9:100            
    gridscale=0.07
    ce_offset=1    # pre-tangential-frame approach  
    ce_scale=1 

elif [ "$GEOM" == "XJfixtureConstructionXZ_1" ]; then

    note="this view is a good one : clearly see side cross section of sTarget sAcrylic sXJfixture sXJanchor  "
    moi="solidXJfixture:10"
    cegs=16:0:9:100           
    gridscale=0.20

    ce_offset=1    # pre-tangential-frame approach  
    ce_scale=1 

elif [ "$GEOM" == "XJfixtureConstructionYZ_1" ]; then

    note="this view is difficult to interpret : could be a bug or just a slice at a funny angle, need tangential check"
    moi="solidXJfixture:10"
    cegs=0:16:9:100            
    gridscale=0.20

    ce_offset=1    # pre-tangential-frame approach  
    ce_scale=1 

elif [ "$GEOM" == "XJfixtureConstructionTP_1" ]; then

    note="nicely aligned rectangle in YZ=(TP), longer in T direction +- 1 extent unit, +-0.24 extent units in P  "
    moi="solidXJfixture:10:-3"
    cegs=0:16:9:100            
    gridscale=0.20

elif [ "$GEOM" == "XJfixtureConstructionRT_10" ]; then

    note="bang on tangential view from P(phi-tangent-direction) showing the radial coincidence issues clearly" 
    moi="solidXJfixture:10:-3"
    cegs=16:9:0:100            
    gridscale=0.20

elif [ "$GEOM" == "XJfixtureConstructionRP_10" ]; then

    note="bang on tangential view from T(theta-tangent-direction) radial coincidences are clear, also another in P direction apparent"
    moi="solidXJfixture:10:-3"
    cegs=16:0:9:100            
    gridscale=0.20

elif [ "$GEOM" == "XJfixtureConstructionRP_55" ]; then

    moi="solidXJfixture:55:-3"
    cegs=16:0:9:100               
    #gridscale=0.20
    #gridscale=0.40
    gridscale=0.80

elif [ "$GEOM" == "XJfixtureConstructionPR_55" ]; then

    moi="solidXJfixture:55:-3"
    #    R:T:P        larger side of grid becomes horizontal : hence  PR  (not RP)
    cegs=9:0:16:100          
    #gridscale=0.20
    #gridscale=0.40
    gridscale=0.80

elif [ "$GEOM" == "XJfixtureConstructionTR_55" ]; then

    moi="solidXJfixture:55:-3"
    #    R:T:P        larger side of grid becomes horizontal : hence  TR  (not RT)
    cegs=9:16:0:100            
    #gridscale=0.20
    #gridscale=0.40
    gridscale=0.80


elif [ "$GEOM" == "25" ]; then
    cfbase=$TMP/CSGDemoTest/dcyl    
    moi=0
    cegs=16:0:9:100
    gridscale=0.025
    isel=0                           # setting isel to zero, prevents skipping bnd 0 
elif [ "$GEOM" == "30" ]; then
    note="HMM : box minus sub-sub cylinder NOT showing the spurious intersects, maybe nice round demo numbers effect"
    cfbase=$TMP/CSGDemoTest/bssc    
    moi=0
    cegs=16:0:9:100
    gridscale=0.025
    isel=0                           # setting isel to zero, prevents skipping bnd 0 
elif [ "$GEOM" == "100" ]; then
    cfbase=$TMP/GeoChain/AdditionAcrylicConstruction  
    moi=0
    cegs=16:0:9:100
    gridscale=0.1
    isel=0
elif [ "$GEOM" == "101" ]; then
    cfbase=$TMP/GeoChain/BoxMinusTubs1
    moi=0
    cegs=16:0:9:100
    gridscale=0.1
    isel=0

elif [ "$GEOM" == "SphereWithPhiSegment" ]; then
    cfbase=$TMP/GeoChain/$GEOM
    num_pho=100
    cegs=9:16:0:0:0:$dz:$num_pho
    gridscale=0.10

else
    # everything else assume single PMT dimensions
    cfbase=$TMP/GeoChain/$GEOM
    moi=0
    dz=-4
    num_pho=100
    #cegs=16:0:9:0:0:$dz:$num_pho
    cegs=9:0:16:0:0:$dz:$num_pho
    #gridscale=0.15
    gridscale=0.10
    isel=
    unset CXS_OVERRIDE_CE
    export CXS_OVERRIDE_CE=0:0:-130:320   ## fix at the full uncut ce 

fi 

if [ "$(uname)" == "Linux" ]; then
    if [ -n "$cfbase" -a ! -d "$cfbase/CSGFoundry" ]; then
       echo $msg : ERROR : cfbase directory $cfbase MUST contain CSGFoundry subfolder 
       echo $msg : TIPS : run GeoChain first to create the geometry and use b7 to build CSGOptiX 
       exit 1 
    fi 
fi

export MOI=${MOI:-$moi}
export CXS_CEGS=${CXS_CEGS:-$cegs}
export CE_OFFSET=${CE_OFFSET:-$ce_offset}
export CE_SCALE=${CE_SCALE:-$ce_scale}
export GRIDSCALE=${GRIDSCALE:-$gridscale}
export TOPLINE="cxs.sh CSGOptiXSimulateTest CXS $CXS MOI $MOI CXS_CEGS $CXS_CEGS GRIDSCALE $GRIDSCALE ISEL $ISEL"
export BOTLINE="ZOOM $ZOOM LOOK $LOOK ZZ $ZZ XX $XX GEOM $GEOM "
export GSPLOT=${GSPLOT:-$gsplot}

if [ -n "$cfbase" ]; then 
    echo $msg cfbase $cfbase defined setting CFBASE to override standard geometry default 
    export CFBASE=${CFBASE:-$cfbase}   ## setting CFBASE only appropriate for non-standard geometry 
fi 

export ISEL=${ISEL:-$isel}
export XX=${XX:-$xx}
export ZZ=${ZZ:-$zz}

export OPTICKS_GEOM=$GEOM 

vars="BASH_FOLDER MOI CE_OFFSET CE_SCALE CXS_CEGS CXS_OVERRIDE_CE GRIDSCALE TOPLINE BOTLINE GSPLOT ISEL XX ZZ FOLD OPTICKS_GEOM OPTICKS_RELDIR"
for var in $vars ; do printf "%20s : %s \n" $var ${!var} ; done 


pkg=CSGOptiX
bin=CSGOptiXSimulateTest 
export LOGDIR=/tmp/$USER/opticks/$pkg/$bin
mkdir -p $LOGDIR 
cd $LOGDIR 


if [ "$(uname)" == "Linux" ]; then 

    if [ "$1" == "run" ]; then

        $GDB CSGOptiXSimulateTest
        source CSGOptiXSimulateTest_OUTPUT_DIR.sh || exit 1  

    elif [ "$1" == "ana" ]; then 

        source CSGOptiXSimulateTest_OUTPUT_DIR.sh || exit 1  
        NOGUI=1 ${IPYTHON:-ipython} ${BASH_FOLDER}/tests/CSGOptiXSimulateTest.py 

    else

        $GDB CSGOptiXSimulateTest
        source CSGOptiXSimulateTest_OUTPUT_DIR.sh || exit 1  
        NOGUI=1 ${IPYTHON:-ipython} ${BASH_FOLDER}/tests/CSGOptiXSimulateTest.py 

    fi

elif [ "$(uname)" == "Darwin" ]; then

    source CSGOptiXSimulateTest_OUTPUT_DIR.sh || exit 1  
    echo $msg CSGOptiXSimulateTest_OUTPUT_DIR $CSGOptiXSimulateTest_OUTPUT_DIR

    if [ "$1" == "bat" ]; then
        NOGUI=1 ${IPYTHON:-ipython} --pdb -i ${BASH_FOLDER}/tests/CSGOptiXSimulateTest.py 
    else
        ${IPYTHON:-ipython} --pdb -i ${BASH_FOLDER}/tests/CSGOptiXSimulateTest.py 
    fi 
fi 

exit 0
