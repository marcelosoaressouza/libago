#!/bin/bash

CFLAGS="-g -O0 -Wall -Wextra"

AGO="`pwd`"

INCLUDES="-I$AGO/src/include -I/usr/include"

LIBS="-L$AGO/lib -L/usr/lib -lagoft -lagolog -lagomatrix -lmpich"

CONFIGURA_MPICC="$CFLAGS $INCLUDES $LIBS"

rm $AGO/saida/ago 2> /dev/null

/usr/bin/mpicc $CONFIGURA_MPICC \
               $AGO/src/main.c \
               $AGO/src/agocore.c \
               $AGO/src/agomaster.c \
               $AGO/src/agoworker.c \
               $AGO/src/fault/agocorefault.c \
               $AGO/src/fault/agomasterfault.c \
               $AGO/src/fault/agoworkerfault.c \
               -o $AGO/saida/ago

echo `date` > $AGO/saida/ultima_compilacao_ago

