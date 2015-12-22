#!/bin/bash

source setenv.sh

# This script will read 1000 las files from a directory
echo "Starting File Utility Test"

time ./testFileUtils /projects/isgs/lidar/adams/las_classified/ 1000

echo "Ending File Utility Test"
