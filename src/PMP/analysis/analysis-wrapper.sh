#!/bin/bash

TARGET=`echo $1 | sed 's#.*/##g'`

ROOT=$HOME/ForceExecution
WORKDIR=$ROOT/work/$TARGET

export ROOT=$ROOT
export WORKDIR=$WORKDIR

ANALYSIS=$ROOT/analysis
DYNINST=$ROOT/Dyninst

export LD_LIBRARY_PATH=$DYNINST/exe/lib:$DYNINST/build/tbb/lib:$DYNINST/build/elfutils/lib:$LD_LIBRARY_PATH
export DYNINSTAPI_RT_LIB=$DYNINST/exe/lib/libdyninstAPI_RT.so

$ANALYSIS/analysis $@
