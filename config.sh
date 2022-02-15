#!/bin/bash

# Provides an example of how to configure Partix with cmake
# ---------------------------------------------------------

cd build
cmake ..  -DCMAKE_C_COMPILER=mpicc -DCMAKE_LINKER=mpicc -DQthreads_ROOT=$QTHREADS_INSTALL_PATH -DPartix_ENABLE_QTHREADS=ON
