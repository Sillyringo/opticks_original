#!/bin/bash -l
##
## Copyright (c) 2019 Opticks Team. All Rights Reserved.
##
## This file is part of Opticks
## (see https://bitbucket.org/simoncblyth/opticks).
##
## Licensed under the Apache License, Version 2.0 (the "License"); 
## you may not use this file except in compliance with the License.  
## You may obtain a copy of the License at
##
##   http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software 
## distributed under the License is distributed on an "AS IS" BASIS, 
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
## See the License for the specific language governing permissions and 
## limitations under the License.
##

VERBOSE=1
[ "$0" == "$BASH_SOURCE" ] && sauce=0 || sauce=1

o-(){    . $(which o.sh) ; } 
o-vi(){ vi $(which o.sh) ; } 

cmdline="$*"

o-usage(){ cat << \EOU
/**
o.sh
======

Executables are selected by corresponding arguments:

--oktest OKTest
    full Opticks simulation and visualization without Geant4

--okg4test OKG4Test
    full Opticks and Geant4 simulation and visualization with comparison

--okx4test OKX4Test
    direct translation of GDML geometry to Opticks geocache

--g4oktest G4OKTest
    testing G4Opticks interface used from detector frameworks

--ggeotesttest GGeoTestTest
    pre-GPU machinery testing 


* Gathering the useful parts of op.sh prior to dumping that.

**/

EOU
}

o-cmdline-parse()
{
   local msg="=== $FUNCNAME $VERBOSE :"
   [ -n "$VERBOSE" ] && echo $msg START 

   o-cmdline-specials
   o-cmdline-binary-match
   o-cmdline-binary

   [ -n "$VERBOSE" ] && echo $msg DONE
   [ -n "$VERBOSE" ] && o-cmdline-info
}


o-binary-name-default(){ echo OKG4Test ; }
o-binary-names(){ type o-binary-name | perl -ne 'm,--(\w*)\), && print "$1\n" ' - ; } 
o-binary-name()
{
   case $1 in 
         --oktest) echo OKTest ;;
       --okg4test) echo OKG4Test ;;
       --okx4test) echo OKX4Test ;;
       --g4oktest) echo G4OKTest ;;
         --tracer) echo OTracerTest ;;
   --ggeotesttest) echo GGeoTestTest ;;
   esac 
   # no default as its important this return blank for unidentified commands
}

o-cmdline-specials()
{
   local msg="=== $FUNCNAME $VERBOSE :"
   [ -n "$VERBOSE" ] && echo $msg 

   unset OPTICKS_DBG 
   unset OPTICKS_LOAD
   unset OPTIX_API_CAPTURE

   if [ "${cmdline/--malloc}" != "${cmdline}" ]; then
       export OPTICKS_MALLOC=1
   fi

   if [ "${cmdline/--debugger}" != "${cmdline}" ]; then
       export OPTICKS_DBG=1
   elif [ "${cmdline/-D}" != "${cmdline}" ]; then 
       export OPTICKS_DBG=1
   elif [ "${cmdline/-DD}" != "${cmdline}" ]; then
       export OPTICKS_DBG=1
       export OPTICKS_LLDB_SOURCE=/tmp/g4lldb.txt
   elif [ "${cmdline/--strace}" != "${cmdline}" ]; then
       export OPTICKS_DBG=2
   fi

   if [ "${cmdline/--load}" != "${cmdline}" ]; then
       export OPTICKS_LOAD=1
   fi
   if [ "${cmdline/--oac}" != "${cmdline}" ]; then
       export OPTIX_API_CAPTURE=1
   fi
   [ -n "$VERBOSE" ] && echo $msg 
}

o-cmdline-binary-match()
{
    local msg="=== $FUNCNAME $VERBOSE :"
    [ -n "$VERBOSE" ] && echo $msg finding 1st argument with associated binary 

    local arg
    local bin
    unset OPTICKS_CMD

    for arg in $cmdline 
    do
       bin=$(o-binary-name $arg)
       #echo arg $arg bin $bin  
       if [ "$bin" != "" ]; then 
           export OPTICKS_CMD=$arg
           echo $msg $arg
           return 
       fi
    done
}

o-cmdline-binary()
{
   unset OPTICKS_BINARY 
   unset OPTICKS_ARGS

   local cfm=$OPTICKS_CMD
   local bin=$(o-binary-name $cfm) 
   local def=$(o-binary-name-default)

   if [ "$bin" == "" ]; then
      bin=$def
   fi 

   export OPTICKS_BINARY=$(opticks-prefix)/lib/$bin   # do not assume PATH is setup 
   export OPTICKS_ARGS=$cmdline
}

o-cmdline-info(){ cat << EOI

     OPTICKS_CMD    : $OPTICKS_CMD 
     OPTICKS_BINARY : $OPTICKS_BINARY
     OPTICKS_ARGS   : $OPTICKS_ARGS

EOI
}

o-gdb-update()
{
   local msg="=== $FUNCNAME :" 
   echo $msg placeholder
}


o-lldb-update()
{
   # needs macOS system python with lldb python module
   local msg="=== $FUNCNAME :" 
   if [ -n "${OPTICKS_DBG}" -a -n "${OPTICKS_LLDB_SOURCE}" ] ; then  
      echo $msg Updating OPTICKS_LLDB_SOURCE with stdout from g4lldb.py : ${OPTICKS_LLDB_SOURCE}
      echo "run" > ${OPTICKS_LLDB_SOURCE}.autorun
      g4lldb.py > ${OPTICKS_LLDB_SOURCE}
   fi 
}

o-lldb-dump()
{
   local msg="=== $FUNCNAME :" 
   if [ -n "${OPTICKS_DBG}" -a -n "${OPTICKS_LLDB_SOURCE}" -a -f "${OPTICKS_LLDB_SOURCE}" ]; then 
        echo $msg Active OPTICKS_LLDB_SOURCE : ${OPTICKS_LLDB_SOURCE}
        ls -l "${OPTICKS_LLDB_SOURCE}"
        cat "${OPTICKS_LLDB_SOURCE}"
   fi   
}


o-lldb-identify(){
   : macOS has some security that prevents /usr/bin/lldb from seeing DYLD_LIBRARY_PATH envvar
   : however direct use of the lldb binary from within the Xcode bundle does not suffer from this restriction

   local lldb_bin=/Applications/Xcode/Xcode_10_1.app/Contents/Developer/usr/bin/lldb
   [ ! -f "$lldb_bin" ] && echo $FUNCNAME : WARNING lldb_bin $lldb_bin DOES NOT EXIST && return 1 
   export LLDB=$lldb_bin 
   return 0 
}


o-lldb-runline-old()
{
 
   if [ -n "${OPTICKS_LLDB_SOURCE}" -a -f "${OPTICKS_LLDB_SOURCE}" ] ; then 
      #echo lldb -f ${OPTICKS_BINARY} -s ${OPTICKS_LLDB_SOURCE} -s ${OPTICKS_LLDB_SOURCE}.autorun -- ${OPTICKS_ARGS} 
      # autorun manages to launch the process but output does not arrive in lldb console, 
      # and seemingly input doesnt get to the the app
      echo $LLDB -f ${OPTICKS_BINARY} -s ${OPTICKS_LLDB_SOURCE} -- ${OPTICKS_ARGS} 
   else
      echo $LLDB -f ${OPTICKS_BINARY} -- ${OPTICKS_ARGS} 
   fi 
}



o-lldb-runline()
{
   local H
   local B
   local T

   if [ -n "${OPTICKS_LLDB_SOURCE}" -a -f "${OPTICKS_LLDB_SOURCE}" ] ; then 
       H="-s ${OPTICKS_LLDB_SOURCE}";
   else 
       H=""
   fi 
   if [ -z "$BP" ]; then
        B="";
    else
        B="";
        for bp in $BP
        do
            B="$B -o \"b $bp\" "
        done
        B="$B -o b"
    fi;
    T="-o r"

    local runline="$LLDB -f ${OPTICKS_BINARY} $H $B $T -- ${OPTICKS_ARGS}"
    echo $runline 
}



o-gdb-runline()
{
   local H
   local B
   local T
   if [ -z "$BP" ]; then
        H="";
        B="";
        T="-ex r";
    else
        H="-ex \"set breakpoint pending on\"";
        B="";
        for bp in $BP;
        do
            B="$B -ex \"break $bp\" ";
        done;
        T="-ex \"info break\" -ex r";
    fi;
    local runline="gdb $H $B $T --args ${OPTICKS_BINARY} ${OPTICKS_ARGS}"
    echo $runline 
}


o-runline-notes(){ cat << EON

Use "--debugger" option to set the intername envvar OPTICKS_DBG

EON
}

o-runline()
{
   [ "$(uname)" == "Darwin" ] && o-lldb-identify 

   local runline
   if [ "${OPTICKS_BINARY: -3}" == ".py" ]; then
      runline="python ${OPTICKS_BINARY} ${OPTICKS_ARGS} "
   elif [ "${OPTICKS_DBG}" == "1" ]; then 
      case $(uname) in
          Darwin) runline=$(o-lldb-runline) ;;
           MING*) runline="     ${OPTICKS_BINARY} -- ${OPTICKS_ARGS} " ;; 
               *) runline=$(o-gdb-runline) ;; 
      esac
   elif [ "${OPTICKS_DBG}" == "2" ]; then 
      runline="strace -o /tmp/strace.log -e open ${OPTICKS_BINARY} ${OPTICKS_ARGS}" 
   else
      runline="${OPTICKS_BINARY} ${OPTICKS_ARGS}" 
   fi
   echo $runline
}

o-postline()
{
   local postline
   if [ "${OPTICKS_DBG}" == "2" ]; then 
       postline="$OPTICKS_PREFIX/bin/strace.py -f O_CREAT"  
   else
       postline="echo $FUNCNAME : dummy"
   fi
   echo $postline 
}


o-malloc()
{
   export MallocStackLoggingNoCompact=1   # all allocations are logged
   export MallocScribble=1     # free sets each byte of every released block to the value 0x55.
   export MallocPreScribble=1  # sets each byte of a newly allocated block to the value 0xAA
   export MallocGuardEdges=1   # adds guard pages before and after large allocations
   export MallocCheckHeapStart=1 
   export MallocCheckHeapEach=1 
}
o-unmalloc()
{
   unset MallocStackLoggingNoCompact
   unset MallocScribble
   unset MallocPreScribble
   unset MallocGuardEdges
   unset MallocCheckHeapStart
   unset MallocCheckHeapEach
}


o-paths(){
   local vars="OPTICKS_MODE OPTICKS_TOP CMAKE_PREFIX_PATH PKG_CONFIG_PATH PATH LD_LIBRARY_PATH DYLD_LIBRARY_PATH"
   local var 
   for var in $vars ; do
       echo $var
       echo ${!var} | tr ":" "\n"
       echo
   done
}

export RC=0 
o-main()
{
   local msg="=== $FUNCNAME :"
   o-cmdline-parse
    
   [ "$(uname)" == "Linux" ] && o-gdb-update
   [ "$(uname)" == "Darwin" ] && o-lldb-update

   local runline=$(o-runline)
   local postline=$(o-postline)

   # setup here or rely on oe- 
   source $OPTICKS_PREFIX/bin/opticks-setup.sh 
   #[ -n "$VERBOSE" ] && o-paths

   echo $msg $runline ======= PWD $PWD $(date)
   eval $runline
   RC=$?
   echo $msg runline PWD $PWD  RC $RC $(date)
   echo $runline  

   #[ $RC -eq 0 ] && echo $postline && eval $postline 
   echo $postline && eval $postline 

   echo PWD : $PWD 
   ls -l *.log 
}

o-main
echo $0 : RC : $RC 
exit $RC
