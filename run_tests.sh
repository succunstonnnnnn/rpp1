#!/bin/bash

SIZES=(10000 100000 1000000 10000000 100000000)
PROCS=(2 4 8)

echo "Запуск тестів"
echo ""

for N in "${SIZES[@]}"; do
    echo "Розмір N = $N"

    echo ""
    echo ">>> Послідовна версія:"
    ./int_seq $N | grep -E "method|Time|Error"

    for P in "${PROCS[@]}"; do
        echo ""
        echo ">>> MPI з $P процесами:"
        mpirun --oversubscribe -np $P ./int_mpi $N | grep -E "method|Time|Error|identical"
    done
    echo ""
done

echo "Готово!"