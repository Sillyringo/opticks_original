#!/bin/bash -l 

defarg="run_mt"
arg=${1:-$defarg}

N=${N:-0} ./U4SimulateTest.sh $arg


