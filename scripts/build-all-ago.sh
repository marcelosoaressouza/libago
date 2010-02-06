#!/bin/bash
AGO="`pwd`"

echo -e "\nCompilando Libs"
sh $AGO/scripts/compilar-libs.sh

echo -e "\nCompilando Ago"
sh $AGO/scripts/compilar-libago.sh

echo -e "\nPronto!"
