#!/bin/bash -l 

dir=/tmp/$USER/opticks/okop/OpFlightPathTest
mkdir -p $dir 

if [ "$1" == "grab" ]; then
    cmd="rsync -rtz --del --progress P:$dir/ $dir/"
    echo $cmd
    eval $cmd
    open $dir
fi


#OPTICKS_FLIGHTPATH_SNAPLIMIT=1000 OpFlightPathTest





