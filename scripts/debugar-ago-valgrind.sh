#!/bin/bash

export MALLOC_CHECK_=1

AGO="`pwd`"
VALGRIND="$AGO/saida/valgrind"
NUM_WORKERS=4

sudo sh $AGO/scripts/matar-ago.sh
sudo sh $AGO/scripts/limpar-cache.sh
sudo sh $AGO/scripts/apagar-log.sh

cd $AGO/saida

echo -e "\ndebugar-ago-valgrind.sh - Iniciando Ago com $NUM_WORKERS"

/usr/bin/mpirun -machinefile $AGO/config/hostworkers -n $NUM_WORKERS \
        /usr/bin/valgrind --tool=callgrind  --trace-children=yes -v --log-file=$VALGRIND/call_log_ago --dump-instr=yes --trace-jump=yes \
        $AGO/saida/ago

#       /usr/bin/valgrind --tool=cachegrind --trace-children=yes -v --log-file=$VALGRIND/cache_log_ago \
#       /usr/bin/valgrind --tool=helgrind   --trace-children=yes -v --log-file=$VALGRIND/hel_log_ago.out \
#       /usr/bin/valgrind --tool=memcheck   --trace-children=yes -v --log-file=$VALGRIND/mem_log_ago --show-reachable=yes --leak-check=full --leak-resolution=high \

echo -e "\ndebugar-ago-valgrind.sh - Finalizada a Execucao de Ago"
cd $AGO

