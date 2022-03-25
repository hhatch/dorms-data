#!/bin/bash
# usage: ./run.sh > run.log 2>&1

# install simulation (run.cpp)
mkdir build
pushd build
  cmake ..
  make
popd

./build/run
