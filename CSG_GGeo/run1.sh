#!/bin/bash -l
usage(){ cat << EOU

ONE_PRIM_SOLID 
    adds extra debugging solids that reuse existing prim one-by-one

EOU
}

    
export ONE_PRIM_SOLID=1,2,3,4
#export ONE_NODE_SOLID=1,2,3,4,8
#export DEEP_COPY_SOLID=1,2,3,4
#export KLUDGE_SCALE_PRIM_BBOX=d1    # d1,d2

./run.sh --gparts_transform_offset 



