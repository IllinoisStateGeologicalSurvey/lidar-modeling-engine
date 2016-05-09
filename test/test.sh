###############################################################
# Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  #
# Information Laboratory (CIGI). All Rights Reserved.         #
#                                                             #
###############################################################

## 
# @file test.sh
# @author Nathan Casler
# @date May 6 2016
# @brief Script to run basic parallel LAS file reading test
#
#
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
