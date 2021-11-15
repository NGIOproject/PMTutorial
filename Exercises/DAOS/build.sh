#!/bin/bash

DAOS=/usr
CC=gcc
CFLAGS="-I$DAOS/include/"
LDFLAGS=" -L$DAOS/lib64 -ldaos -lgurt -g"

FILES="
kv1 kv1_answer
kv2 kv2_answer
kv3 kv3_answer
array1 array1_answer
mkv1 mkv1_answer
"

for f in $FILES
do
    echo "$CC $CFLAGS $f.c $LDFLAGS -o $f"
    $CC $CFLAGS $f.c $LDFLAGS -o $f
done
