#!/bin/bash

#file buildGraph.sh

#SBATCH --job-name=buildGraph
#SBATCH --nodes=9
#SBATCH --ntasks-per-node=2
#SBATCH --output=buildGraph.log

module load mpi/mpich-3.2-x86_64

mpirun /mnt/linuxlab/home/slefever1/COSC420/projects/project2/COSC420proj2/buildGraph

