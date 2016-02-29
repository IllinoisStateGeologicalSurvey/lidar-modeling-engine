#!/bin/bash
#PBS -N las-read
#PBS -A ncasler
#PBS -l walltime=1:00:00
#PBS -l nodes=1:ppn=20

module load zlib
module load hdf5-parallel

# GEt directory of test script
#TESTDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd)"
BASEDIR="/home/ncasler/apps/DSME"
#BASEDIR="$(dirname "${TESTDIR}")"

LASBASE="/projects/isgs/lidar"

BINDIR="${BASEDIR}/bin"
SCRIPTDIR="${BASEDIR}/scripts"
LOGDIR="${BASEDIR}/log"
source ${SCRIPTDIR}/setenv.sh

#export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/gpfs_scratch/ncasler/liblas/lib:/gpfs_scratch/ncasler/proj/lib"
H5NAME="${BASEDIR}/test.h5"
date
echo "---------------------------------------------------------------------"
echo ""
mpirun -np 20 "${BINDIR}/testFileUtils" -i "${LASBASE}/henry/las/" -t 1000 -f "${H5NAME}"  2>&1 | tee ${LOGDIR}/fileUtil.log

echo ""
date
echo "---------------------------------------------------------------------"
echo ""

#mpirun -np 20 "${BINDIR}/headerRead" -i "${H5NAME}" -t 100 2>&1 | tee ${LOGDIR}/header.log
echo "" 
date
echo ""
echo "---------------------------------------------------------------------"
echo " [ FINISHED JOB ]"
