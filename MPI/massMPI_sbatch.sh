#!/bin/bash

for i in 1 2 4 8 12 20 25 32 
do
	 sbatch --mem-per-cpu=2500 --constraint=dwarves --ntasks-per-node=$i --ntasks=$i mpi_sbatch.sh
done
