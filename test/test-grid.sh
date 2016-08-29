############################
# test-grid.sh
# 
# Generate a grid dataset and save it to HDF5 datastore
# ##########################

# @file test-grid.sh
# @author Nathan Casler
# @data June 20 2016
# @brief Script to test HDF5 grid generation and archival

#!/bin/bash

TESTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
BASEDIR="$( dirname "${TESTDIR}")"
LASBASE="/projects/isgs/lidar"

BINDIR="${BASEDIR}/bin"
SCRIPTDIR="${BASEDIR}/scripts"
LOGDIR="${BASEDIR}/log"
DATADIR="${BASEDIR}/data"
#Set environment
source ${SCRIPTDIR}/setenv.sh

echo "Running as ${USER}"
echo "Starting Grid Generation Test"

mpirun -n 1 ${BINDIR}/getGrid 2>&1 | tee ${LOGDIR}/getGrid.log

echo "Ending Grid Generation Test"
