#!/bin/bash -l 

usage(){ cat << EOU
X4MeshTest.sh : Geant4 polyhedron renders using tests/X4MeshTest.cc and tests/X4MeshTest.py
=============================================================================================


GEOM=hmsk_solidMask     EYE=0.5,0.5,-0.3 ZOOM=2 ./X4MeshTest.sh
GEOM=hmsk_solidMaskTail EYE=0.5,0.5,0.3 ZOOM=2 ./X4MeshTest.sh


GEOM=nmsk_solidMaskTail EYE=0.5,0.5,0.3 ZOOM=2 ./X4MeshTest.sh


GEOM=XJfixtureConstruction ./X4MeshTest.sh
GEOM=XJanchorConstruction ./X4MeshTest.sh
GEOM=SJReceiverConstruction ./X4MeshTest.sh


EOU
}


msg="=== $BASH_SOURCE :"

#geom=hmsk_solidMask
#geom=hmsk_solidMaskTail
#geom=nmsk_solidMaskTail
#geom=XJfixtureConstruction 
#geom=AltXJfixtureConstruction 
#geom=XJanchorConstruction
#geom=SJReceiverConstruction
#geom=AnnulusTwoBoxUnion
#geom=AnnulusOtherTwoBoxUnion
#geom=AnnulusFourBoxUnion
geom=GeneralSphereDEV

catgeom=$(cat ~/.opticks/GEOM.txt 2>/dev/null | grep -v \#) && [ -n "$catgeom" ] && geom=$(echo ${catgeom%%_*})
export GEOM=${GEOM:-$geom}

# PYVISTA_KILL_DISPLAY envvar is observed to speedup exiting from ipython after pyvista plotting 
# see https://github.com/pyvista/pyvista/blob/main/pyvista/plotting/plotting.py
export PYVISTA_KILL_DISPLAY=1

outdir="$TMP/extg4/X4MeshTest/$GEOM/X4Mesh"
reldir="/env/presentation/extg4/X4MeshTest/$GEOM/X4Mesh"
pubdir="$HOME/simoncblyth.bitbucket.io$reldir"


case $GEOM in 
   XJfixtureConstruction) source XJfixtureConstruction.sh ;;
   GeneralSphereDEV)   source GeneralSphereDEV.sh ;;
   BoxMinusOrb)        source BoxMinusOrb.sh ;; 
   SJReceiverConstruction) source SJReceiverConstruction.sh ;; 
esac


if [ "$GEOM" == "SJReceiverConstruction" ]; then 
    eye=1,1,0.2 
    zoom=2.5
    export EYE=${EYE:-$eye}
    export ZOOM=${ZOOM:-$zoom}
fi 



dir=$(dirname $BASH_SOURCE)
bin=$(which X4MeshTest)
script=$dir/tests/X4MeshTest.py


echo BASH_SOURCE $BASH_SOURCE bin $bin script $script outdir $outdir GEOM $GEOM


export X4SolidMaker=INFO 

case $GEOM in 
   LHCbRichSphMirr*)  export X4Mesh_noofsides=240  ;;
   LHCbRichFlatMirr*) export X4Mesh_noofsides=48   ;;   ## setting this to high value is too expensive 
esac


$bin
[ $? -ne 0 ] && echo $msg run error && exit 1


echo $msg outdir $outdir
ls -l $outdir 


${IPYTHON:-ipython} --pdb -i $script
[ $? -ne 0 ] && echo $msg ana error && exit 2


if [ -n "$PUB" ]; then 
  echo $msg 
  png=$outdir/pvplot.png 


  if [ ! -d "$pubdir" ] ; then 
     mkdir -p $pubdir
  fi 

  cmd="cp $outdir/pvplot.png $pubdir/pvplot.png" 
  echo $msg $cmd
  eval $cmd 

  s5line="$reldir/pvplot.png" 
  echo $msg s5line $s5line  

fi 


exit 0

