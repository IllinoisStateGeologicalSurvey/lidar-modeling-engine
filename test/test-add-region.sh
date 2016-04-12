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

mpirun -np 20 ${BINDIR}/addRegion -r "jefferson" -p "${LASBASE}/jefferson/LAS" 2>&1 | tee ${LOGDIR}/regionAdd.log


echo "Ending Region Addition"
