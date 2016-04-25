#!/bin/bash

### Sets environmental variables for compilation

module load zlib
module load hdf5-parallel
module load geos

export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/gpfs_scratch/ncasler/liblas/lib:/gpfs_scratch/ncasler/proj/lib"

