#!/bin/bash
AGO="`pwd`"
PARAM="-retvalint -realcompare -compdef -nullpass -unrecog"

splint +posixlib +boolint -booltype BOOLEAN -preproc \
       $1 \
       -I$AGO/src/include -I/usr/include\
       $PARAM
