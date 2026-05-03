#!/bin/bash

SIZES=(10000 100000 1000000 10000000 100000000)
PROCS=(2 4 8)

echo "Запуск тестів"
echo ""

for N in "${SIZES[@]}"; do
    echo "Розмір масиву N = $N"

    echo ""
    echo ">>> Послідовна версія:"
    ./sum_seq $N | grep -E "Time|Sum"

    for P in "${PROCS[@]}"; do
        echo ""
        echo ">>> MPI з $P процесами:"
        mpirun --oversubscribe -np $P ./sum_mpi $N | grep -E "Time|Sum|identical"
    done
    echo ""
done

echo "Готово!"