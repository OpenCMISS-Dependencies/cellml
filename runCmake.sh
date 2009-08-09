#!/bin/sh

# args:
#   -DCMAKE_BUILD_TYPE=Debug
#   -DCMAKE_BUILD_TYPE=Release

export CMAKE_INCLUDE_PATH=$OPENCMISSEXTRAS_ROOT/cellml/x86_64-linux/include
export CMAKE_LIBRARY_PATH=$OPENCMISSEXTRAS_ROOT/cellml/x86_64-linux/lib

cmake $OPENCMISS_ROOT/cellml/basicSimulationApplication $@
