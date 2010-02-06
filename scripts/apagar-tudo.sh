#!/bin/bash 

AGO="`pwd`"

rm -rf $AGO/saida/log/* 2> /dev/null
rm -rf $AGO/saida/valgrind/* 2> /dev/null
rm -rf $AGO/saida/matrizes/* 2> /dev/null
rm -rf $AGO/src/lib/ultima_compilacao_libs 2> /dev/null
rm $AGO/saida/* 2> /dev/null
