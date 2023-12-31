#!/bin/bash -l 
usage(){ cat << EOU
u4s.sh : formerly U4RecorderTest.sh : Geant4 simulation with Opticks recording every random consumption of every step of every photon
=========================================================================================================================================

::

    cd ~/opticks/u4   # u4
    ./u4s.sh 

    BP=DsG4Scintillation::PostStepDoIt ./u4s.sh dbg 

    ./u4s.sh run
    DRYRUN=1 ./u4s.sh run  ## eg to check config of input and output folders 

    ./u4s.sh dbg
    ./u4s.sh clean
    ./u4s.sh ana
    ./u4s.sh ab


U4RecorderTest requires A_CFBASE and A_FOLD envvars to find inputs::

     U4Material::LoadBnd("$A_CFBASE/CSGFoundry/SSim") ; // booting G4Material from A:bnd.npy
   
     sframe fr = sframe::Load_("$A_FOLD/sframe.npy");   // for transforming input photons  
      

U4RecorderTest invokes "SEvt::save()" writing .npy to '$DefaultOutputDir/SEvt::reldir' eg::




EOU
}
msg="=== $BASH_SOURCE :"

case $(uname) in 
   Linux) defarg="run" ;;
   Darwin) defarg="ana" ;; 
esac
arg=${1:-$defarg}

case $arg in 
  fold) QUIET=1 ;;
esac


bin=U4RecorderTest
u4sdir=$(dirname $BASH_SOURCE)
srcdir=$u4sdir
logdir=/tmp/$USER/opticks/$bin
mkdir -p $logdir 
foldbase=$logdir


export DsG4Scintillation_opticksMode=3  # 3:0b11 collect gensteps and do Geant4 generation loop too 

#export DsG4Scintillation_verboseLevel=3
#export DsG4Scintillation_DISABLE=1
#export G4Cerenkov_verboseLevel=3
#export G4Cerenkov_DISABLE=1
#pidx=0
#gidx=117
#export PIDX=${PIDX:-$pidx}
#export GIDX=${GIDX:-$gidx}

export U4Random_select_action=interrupt   ## dumps stack and breaks in debugger to check the process

#mode=gun
#mode=torch
mode=iphoton
export U4RecorderTest__PRIMARY_MODE=$mode

#source ./IDPath_override.sh   
# IDPath_override.sh : non-standard IDPath to allow U4Material::LoadOri to find material properties 
# HMM probably doing nothing now that are using U4Material::LoadBnd ?

source $u4sdir/../bin/GEOM_.sh 

if [ "$U4RecorderTest__PRIMARY_MODE" == "iphoton" ]; then
    source $u4sdir/../bin/OPTICKS_INPUT_PHOTON.sh     
fi 

upfind_cfbase(){
    : traverse directory tree upwards 
    local dir=$1
    while [ ${#dir} -gt 1 -a ! -f "$dir/CSGFoundry/solid.npy" ] ; do dir=$(dirname $dir) ; done 
    echo $dir
}

A_FOLD=$($OPTICKS_HOME/g4cx/gxs.sh fold)
A_CFBASE=$(upfind_cfbase $A_FOLD)  

if [ -z "$A_CFBASE" ]; then 
   echo $msg FAILED TO FIND A_CFBASE starting from A_FOLD $A_FOLD
   exit 1
fi 

export A_FOLD     # A_FOLD is needed for loading "$A_FOLD/sframe.npy" 
export A_CFBASE   # A_CFBASE needed for loading  "$A_CFBASE/CSGFoundry/SSim"

export ShimG4OpAbsorption_FLOAT=1 
export ShimG4OpRayleigh_FLOAT=1 

# cf U4Physics::Desc
physdesc=""
[ -n "$ShimG4OpAbsorption_FLOAT" ] && physdesc="${physdesc}ShimG4OpAbsorption_FLOAT" 
[ -z "$ShimG4OpAbsorption_FLOAT" ] && physdesc="${physdesc}ShimG4OpAbsorption_ORIGINAL" 
physdesc="${physdesc}_"
[ -n "$ShimG4OpRayleigh_FLOAT" ]   && physdesc="${physdesc}ShimG4OpRayleigh_FLOAT"
[ -z "$ShimG4OpRayleigh_FLOAT" ]   && physdesc="${physdesc}ShimG4OpRayleigh_ORIGINAL"

#sel=PIDX_0_
sel=ALL
reldir=$physdesc/$sel     # SEvt::SetReldir 


BASE=$GEOMDIR/$bin
UBASE=${BASE//$HOME\/}    # UBASE is BASE relative to HOME to handle rsync between different HOME
FOLD=$BASE/$reldir

export FOLD

# for GEOMDIR within /tmp there is no need to override the DefaultOutputDir 
# but for GEOMDIR within geocache an override is necessary to keep A_FOLD and B_FOLD together
case $GEOMDIR in 
   $HOME/.opticks/geocache/*) export OPTICKS_OUT_FOLD=$BASE ;;    # SEventConfig::OutFold override SEvt $DefaultOutputDir 
esac

if [ "${arg/fold}" != "${arg}" ]; then 
   echo $FOLD
fi 

if [ "${arg/info}" != "${arg}" ]; then 
    vars="BASH_SOURCE u4sdir GEOM GEOMDIR BASE OPTICKS_OUT_FOLD UBASE CFBASE FOLD A_FOLD A_CFBASE reldir" 
    for var in $vars ; do printf "%30s : %s \n" $var ${!var}  ; done 
    echo 
fi 



OPTICKS_RANDOM_SEQPATH_NOTES(){ cat << EON

The default OPTICKS_RANDOM_SEQPATH in the U4Random code is limited to up to 100k photons
as only a single 100k precooked random file is loaded. 

When simulating more than 100k up to 1M photons it is necessary to uncomment the 
below line to force loading of 10*100k files. 

Note that OPTICKS_RANDOM_SEQPATH uses single quotes to prevent expansion of the '$PrecookedDir' 
which is an SPath internal variable. 

TODO: auto detect when this is needed and do the switch without requiring setting the envvar 

EON
}

export OPTICKS_RANDOM_SEQPATH='$PrecookedDir/QSimTest/rng_sequence/rng_sequence_f_ni1000000_nj16_nk16_tranche100000'  



loglevels()
{
    export Dummy=INFO
    export U4VolumeMaker=INFO
    #export U4Material=INFO
    #export SEvt=INFO
    #export U4Random=INFO
}
loglevels




if [ "${arg/run}" != "${arg}" ]; then 
    cd $logdir 
    $bin 
    [ $? -ne 0 ] && echo $msg run error && exit 1 

    echo $msg logdir $logdir
fi 

if [ "${arg/dbg}" != "${arg}" ]; then 
    cd $logdir 
    case $(uname) in 
       Linux)  gdb_ $bin;;
       Darwin) lldb__ $bin ;;
    esac
    [ $? -ne 0 ] && echo $msg dbg error && exit 2 
    echo $msg logdir $logdir
fi 

if [ "${arg/clean}" != "${arg}" ]; then
   cd $FOLD 
   pwd
   ls -l *.npy *.txt *.log
   read -p "$msg Enter YES to delete these : " ans
   if [ "$ans" == "YES" ] ; then 
       echo $msg proceeding
       rm *.npy *.txt *.log
   else
       echo $msg skip 
   fi 
fi 

if [ "${arg/ana}" != "${arg}" ]; then 
    cd $srcdir 
    pwd
    ${IPYTHON:-ipython} --pdb -i $u4sdir/tests/$bin.py 
fi 

if [ "${arg/grab}" != "${arg}" ]; then 
    echo $msg grab UBASE $UBASE
    source $u4sdir/../bin/rsync.sh $UBASE
fi 

if [ "${arg}" == "ab" ]; then 
    cd $srcdir 
    pwd
    #fold_mode=TMP
    #fold_mode=KEEP
    #fold_mode=LOGF
    fold_mode=GEOM
    export FOLD_MODE=${FOLD_MODE:-$fold_mode}
    source $u4sdir/../bin/AB_FOLD.sh 
    ${IPYTHON:-ipython} --pdb -i $u4sdir/tests/${bin}_ab.py $*  
fi 



