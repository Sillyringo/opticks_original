#!/bin/bash -l

pvn=${SNAP_PVN:-lLowerChimney_phys}
eye=${SNAP_EYE:--1,-1,-1}
emm=${SNAP_EMM:-0}

snapconfig=${SNAP_CFG:-steps=10,ez0=-1,ez1=5}
size=${SNAP_SIZE:-2560,1440,1}

which OpSnapTest 
pwd

snap-cmd(){ cat << EOC
OpSnapTest --targetpvn $pvn --eye $eye --enabledmergedmesh $emm --snapoverrideprefix snap-emm-$emm- $*
EOC
}

cmd=$(snap-cmd $*) 
echo $cmd
eval $cmd
rc=$?
echo rc $rc

 
