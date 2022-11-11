#!/bin/bash

#DAOS=/usr
DAOS=/home/mschaara/install/daos
CC=gcc
CFLAGS="-I$DAOS/include/"
LDFLAGS="-L$DAOS/lib64 -ldaos -ldfs -g"

echo "gcc $CFLAGS -o simple_dfs $LDFLAGS"
gcc $CFLAGS simple_dfs.c -o simple_dfs $LDFLAGS
echo "gcc simple_posix.c -o simple_posix"
gcc simple_posix.c -o simple_posix
echo "mpicc simple_mpiio.c -o simple_mpiio"
mpicc simple_mpiio.c -o simple_mpiio
echo "h5pcc simple_h5.c -o simple_h5 -shlib"
h5pcc simple_h5.c -o simple_h5 -shlib

