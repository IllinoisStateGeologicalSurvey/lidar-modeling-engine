/***************************************************************
 * Copyright (c) 2015-2017 CyberInfrastructure and Geospatial  *
 * Information Laboratory (CIGI). All Rights Reserved.         *
 *                                                             *
 ***************************************************************/

/** 
 * @file point_set.c
 * @author Nathan Casler
 * @date May 6 2016
 * @brief Contains methods for manipulating collections of points
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include "bound.h"
#include "point.h"

point_set PointSet_Create(size_t count, LMEbound extent, LMEcrs crs, LMEpoint* points) {
	point_set pSet;	
	pSet.count = count;
	pSet.extent = extent;
	pSet.crs = crs;
	pSet.points = points;
	return pSet;
}


// TODO: Finish methods for outputting points
// TODO: GET SOME VISUALS!!
// int PointSet_toLAS()
