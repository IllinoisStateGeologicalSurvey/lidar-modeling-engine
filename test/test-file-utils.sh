#!/bin/bash

TESTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
BASEDIR="/home/ncasler/apps/DSME"

LASBASE="/projects/isgs/lidar"

BINDIR="${BASEDIR}/bin"
SCRIPTDIR="${BASEDIR}/scripts"
LOGDIR="${BASEDIR}/log"

# Set environment
source ${SCRIPTDIR}/setenv.sh

H5NAME="${BASEDIR}/test.h5"

# This script will read 1000 las files from a directory
echo "Starting File Utility Test"

mpirun -n 5 ${BINDIR}/testFileUtils -i "${LASBASE}/adams/las_classified/" -t 2000 -f "${H5NAME}" 2>&1 | tee ${LOGDIR}/fileUtil.log

echo "Ending File Utility Test"
