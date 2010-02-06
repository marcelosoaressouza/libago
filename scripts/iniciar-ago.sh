#!/bin/bash

export MALLOC_CHECK_=1

AGO="`pwd`"
NUM_WORKERS=4

sudo sh $AGO/scripts/matar-ago.sh
sudo sh $AGO/scripts/limpar-cache.sh
sudo sh $AGO/scripts/apagar-log.sh

cd $AGO/saida

echo -e "\niniciar-ago.sh - Iniciando Ago com $NUM_WORKERS"

/usr/bin/mpirun -machinefile $AGO/config/hostworkers -n $NUM_WORKERS $AGO/saida/ago

mv d_matrix* matrix*.dat $AGO/saida/matrizes
mv logW* logM* $AGO/saida/log

cd $AGO
echo -e "\niniciar-ago.sh - Finalizada a Execucao de Ago"
