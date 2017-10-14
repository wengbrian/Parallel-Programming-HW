#!/bin/bash
#SBATCH -p batch -N 1 -n 12
mpicxx ./test_cumm.cc
srun -n 2 ./a.out
