#!/bin/bash -l 

CMD=$1
usage(){ cat << EOU
evtsync.sh 
===========

rsyncs OpticksEvent .npy arrays and .json metadata between machines 

::

   PFX=tds3ip evtsync.sh  
   PFX=tds3gun evtsync.sh  

   PFX=tds3gun evtsync.sh rm     # deletes the destination dir before sync, for cleanliness
   PFX=tds3gun evtsync.sh ls     # skips the rsync, justs lists
 

Example directories that are synced::

    evtsync from P:/tmp/blyth/opticks/tds3gun/evt/g4live/natural to /tmp/blyth/opticks/tds3gun/evt/g4live/natural

 
EOU
}


REMOTE=${REMOTE:-P}
#OPTICKS_EVENT_BASE_REMOTE=${OPTICKS_EVENT_BASE_REMOTE:-/home/$USER/local/opticks/evtbase}
OPTICKS_EVENT_BASE_REMOTE=${OPTICKS_EVENT_BASE_REMOTE:-/tmp/$USER/opticks}
OPTICKS_EVENT_BASE=${OPTICKS_EVENT_BASE:-/tmp/$USER/opticks}

evtsync()
{
   local msg="=== $FUNCNAME :"
   local reldir=${1}
   [ -z "$reldir" ] && echo $msg ERROR missing reldir arg && return 1 

   local from=$REMOTE:${OPTICKS_EVENT_BASE_REMOTE}/$reldir
   local to=${OPTICKS_EVENT_BASE}/$reldir

    echo $FUNCNAME from $from to $to

    if [ "$CMD" == "rm" -a -d "$to" ]; then 
       rm -rf $to   
    fi
    mkdir -p $to 

    local rc

    if [ "$CMD" != "ls" ]; then
        rsync -zarv --progress --include="*/" --include="*.npy" --include="*.txt" --include="*.json" --include="*.ini" --exclude="*" "${from}/" "${to}/"
        rc=$? 
        if [ $rc -ne 0 ]; then 
            echo $msg non-zero rc from rsync : start ssh tunnel with \"tun\" and ensure remote directory being grabbed exists && exit 1
        fi 
    fi 
    ls -1rt `find ${to%/} -name '*.npy' `

    return 0   
}



PFX=${PFX:-tds3gun}
reldir=${PFX}/evt/g4live/natural 

evtsync ${reldir}


