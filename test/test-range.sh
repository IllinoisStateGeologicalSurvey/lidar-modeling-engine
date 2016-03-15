#!/bin/bash


module load zlib
module load hdf5-parallel

BASEDIR="/home/ncasler/apps/DSME"

LASBASE="/projects/isgs/lidar"

BINDIR="${BASEDIR}/bin"
SCRIPTDIR="${BASEDIR}/scripts"
LOGDIR="${BASEDIR}/log"

source ${SCRIPTDIR}/setenv.sh

H5NAME="${BASEDIR}/test.h5"
date
echo "-----------------------------------------------------------------"
echo ""

${BINDIR}/testRange -b1 -88.26681 41.98540 -88.25840 41.99156 -r "dupage"  2>&1 | tee ${LOGDIR}/range.log

echo ""
echo "-----------------------------------------------------------------"
echo " [ FINISHED JOB ] "
