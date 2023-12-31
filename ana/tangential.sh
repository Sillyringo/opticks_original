#!/bin/bash -l

msg="=== $BASH_SOURCE :"

${IPYTHON:-ipython} -i tangential.py 


tangential_cc_test()
{
    local name=tangential
    local bin=/tmp/$USER/opticks/ana/$name
    mkdir -p $(dirname $bin)

    gcc $name.cc \
        -I${OPTICKS_PREFIX}/externals/glm/glm \
        -std=c++11 \
        -lstdc++ \
        -o ${OPTICKS_PREFIX}/lib/$name 

    [ $? -ne 0 ] && echo $msg compile error && exit 1 

    which $name
    $name
    [ $? -ne 0 ] && echo $msg run error && exit 2 
}

#tangential_cc_test



exit 0


