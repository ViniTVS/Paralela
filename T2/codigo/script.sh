#!/bin/bash

FILE_DIR="../../testes/"
num_threads=(2 1)

declare -a arquivos=("${FILE_DIR}100000.in ${FILE_DIR}200000.in2" 
    "${FILE_DIR}200000.in ${FILE_DIR}200000.in2" 
    "${FILE_DIR}200000.in ${FILE_DIR}400000.in"
)

eval "rm -rf teste"
eval "mkdir -p teste"

for m in $(seq 1 1 20)
do
    for i in ${num_threads[@]}
    do
        for ((j = 0; j < ${#arquivos[@]}; j++))
        do
            mpirun -np $i --use-hwthread-cpus lcs ${arquivos[$j]}  >> teste/${i}_${j}.txt
        done
    done
done