#!/bin/bash

module load zlib
module load hdf5-parallel

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/gpfs_scratch/ncasler/liblas/lib:/gpfs_scratch/ncasler/proj/lib"

gdb --args mpirun -np 2 ./readLas -i /projects/isgs/lidar/dupage/las/POINTS_001.las
