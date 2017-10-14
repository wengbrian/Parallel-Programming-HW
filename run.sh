#!/bin/bash
#SBATCH -p batch -N 1 -n 12
mpicxx ./test.cc
time srun -n 1 ./a.out 15 ./samples/testcase02 ./ans/testcase02ans 
