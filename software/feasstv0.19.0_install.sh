#!/bin/bash
# usage: ./install.sh > install.log 2>&1

# install feasst
for version in v0.19.0; do
  git clone https://github.com/usnistgov/feasst feasst$version
  pushd feasst$version
    git checkout $version
    mkdir build
    pushd build
      cmake ..
      make -j4 install
    popd
  popd
done
