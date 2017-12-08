#!/bin/sh

BASEDIR=$(dirname "$0")

echo $BASEDIR

whoami

cd $BASEDIR
mkdir -p build
rm -rf build/*
cd build
cmake ..
make
#make test
./src/sched_setattr
