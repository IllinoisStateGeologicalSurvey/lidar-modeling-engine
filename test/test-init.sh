#!/bin/bash

TESTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
BASEDIR="/home/ncasler/apps/DSME"

LASBASE="/projects/isgs/lidar"

BINDIR="${BASEDIR}/bin"
SCRIPTDIR="${BASEDIR}/scripts"
LOGDIR="${BASEDIR}/log"

#set environment
source ${SCRIPTDIR}/setenv.sh


echo "Initializing the data store"

${BINDIR}/initLME 2>&1 | tee ${LOGDIR}/init.log

echo "Ending Init Script"
