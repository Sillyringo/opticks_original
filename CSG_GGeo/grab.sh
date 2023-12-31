#!/bin/bash -l 

pkg=$(basename $(pwd))
#pkg="outdir"

from=P:/tmp/$USER/opticks/$pkg/
to=/tmp/$USER/opticks/$pkg/

mkdir -p $to 

echo pkg $pkg from $from to $to

if [ "$1" != "ls" ]; then
rsync -zarv --progress --include="*/" --include="*.txt" --include="*.npy" --include="*.jpg" --include="*.mp4" --include "*.json" --exclude="*" "$from" "$to"
fi 

ls -1rt `find ${to%/} -name '*.json' `
ls -1rt `find ${to%/} -name '*.jpg' -o -name '*.mp4' -o -name '*.npy'  `


