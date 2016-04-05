#!/bin/bash

TESTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
BASEDIR="/home/ncasler/apps/DSME"

LASBASE="/projects/isgs/lidar"

BINDIR="${BASEDIR}/bin"
SCRIPTDIR="${BASEDIR}/scripts"
LOGDIR="${BASEDIR}/log"

#Set environment
source ${SCRIPTDIR}/setenv.sh

echo "Starting Region Addition"

	mpirun -np 4 ${BINDIR}/addRegion -r "clark" -p "${LASBASE}/clark/las" 2>&1 | tee ${LOGDIR}/regionAdd.log


echo "Ending Region Addition"
