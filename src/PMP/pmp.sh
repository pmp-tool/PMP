#!/bin/bash

TARGET=`echo $1 | sed 's#.*/##g'`

ROOT=$HOME/PMP
BAK=$ROOT/bak
WORKDIR=$ROOT/work/$TARGET


do_init() {

  echo 0 > /proc/sys/vm/mmap_min_addr
  echo 0 > /proc/sys/kernel/randomize_va_space

  rm -rf $WORKDIR
  mkdir $WORKDIR

  cp $BAK/* $WORKDIR

  export ROOT=$ROOT
  export WORKDIR=$WORKDIR

}


do_explore() {

  $ROOT/PMP $@

}


do_init
do_explore $@
