###############################################################
# Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  #
# Information Laboratory (CIGI). All Rights Reserved.         #
#                                                             #
###############################################################

## 
# @file test-read.sh
# @author Nathan Casler
# @date May 6 2016
# @brief Script to run LAS file reading tests
#
#
#!/bin/bash

TESTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
BASEDIR="/home/ncasler/apps/DSME"
LASBASE="/projects/isgs/lidar"

BINDIR="${BASEDIR}/bin"
SCRIPTDIR="${BASEDIR}/scripts"
LOGDIR="${BASEDIR}/log"
DATADIR="${BASEDIR}/data"

# Set environment
source ${SCRIPTDIR}/setenv.sh

echo "Running as ${USER}"
echo "Starting Point Read Test"

mpirun -n 10 ${BINDIR}/readPoints -i ${DATADIR}/files.txt -b -88.26681 41.98540 -88.25840 41.99156 2>&1 | tee ${LOGDIR}/readPoints.log


echo "Ending Point Read Test"
