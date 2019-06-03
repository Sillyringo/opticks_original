#!/bin/bash -l

arg=${1:-box}

cd /tmp

echo ====== $0 $* ====== PWD $PWD ========= arg $arg ========

tboolean-
cmd="tboolean-$arg --okg4 --compute --strace --dbgemit --args"
echo $cmd
eval $cmd
rc=$?

strace.py -f O_CREAT


echo ====== $0 $* ====== PWD $PWD ========= arg $arg ======== RC $rc =======

exit $rc
