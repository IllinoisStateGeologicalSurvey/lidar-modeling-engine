###############################################################
# Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  #
# Information Laboratory (CIGI). All Rights Reserved.         #
#                                                             #
###############################################################

## 
# @file test-add-region.sh
# @author Nathan Casler
# @date May 6 2016
# @brief Script to run region addition test
#
#
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
