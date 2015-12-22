#!/bin/bash
### Set the name of the job
#PBS -N pSort

### set the shell to use
#PBS -S /bin/bash

#PBS -e /home/ncasler/apps/DSME/log/pSort.err
#PBS -o /home/ncasler/apps/DSME/log/pSort.log


### Set the number of nodes/processors 
#PBS -l nodes=1:ppn=20

### Set the maximum wall time
#PBS -l walltime=1:00:00

### use the submision environment
###PBS -V

# Start job from the directory it was submitted

#####################################################
### Load the modules
#####################################################
module load mpich

## Start job from the directory it was submitted
cd $PBS_0_WORKDIR
echo "Working directory: ${PBS_0_WORKDIR}"

mpirun=$(which mpirun)
echo "MPIRUN located at: ${mpirun}"
execDir=/home/ncasler/apps/DSME




mpirun -n $PBS_NP ${execDir}/mpi_mm


