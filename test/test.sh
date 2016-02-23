#!/bin/bash

module load zlib
module load hdf5-parallel


TESTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
BASEDIR="$(dirname "${TESTDIR}")"
SCRIPTDIR="${BASEDIR}/scripts"
BINDIR="${BASEDIR}/bin"


LASBASE="/projects/isgs/lidar"

source ${SCRIPTDIR}/setenv.sh
echo "Reading ${LASBASE}/dupage/las/POINTS_001.las"
gdb --args mpirun -np 2 ${BINDIR}/readLas -i "${LASBASE}/dupage/las/POINTS_001.las" 
