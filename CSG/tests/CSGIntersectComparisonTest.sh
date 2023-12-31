#!/bin/bash -l 
usage(){ cat << EOU
CSGIntersectComparisonTest.sh 
===============================

::

    NUM=1000000 ./CSGIntersectComparisonTest.sh 

EOU
}

bin=CSGIntersectComparisonTest
defarg="run_ana"
arg=${1:-$defarg}


if [ "${arg/run}" != "$arg" ]; then 

    export EPSILON=1e-6

    $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE run error && exit 1 
fi 

if [ "${arg/ana}" != "$arg" ]; then 

    export FOLD=/tmp/$USER/opticks/$bin
    ${IPYTHON:-ipython} --pdb -i $(dirname $BASH_SOURCE)/$bin.py 
    [ $? -ne 0 ] && echo $BASH_SOURCE ana error && exit 2 
fi 

exit 0 

