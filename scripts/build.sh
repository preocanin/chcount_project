#!/usr/bin/bash

# BUILD CLI
pushd cli
mkdir build 2>/dev/null
pushd build
cmake ..
make -j
popd
popd

#BUILD BACKEND
pushd backend
mkdir build 2>/dev/null
pushd build
cmake ..
make -j
popd
popd
