#include <stdlib.h>
#include <stdio.h>
#include <gsl/gsl_math.h>
#include "bound.h"
#include "coord.h"
#include "qtree.h"


int main(int argc, char* argv[]){
	double x, y, z;
	LMEcoord low, high;
	LMEbound bounds;
	LMEquadTree tree;
	
	x = 140.0;
	y = 40.0;
	z = 100;

	LMEcoord_set(&low, x,y,z);
	x = 160.0;
	y = 80.0;
	z = 5000.0;
	LMEcoord_set(&high, x,y,z);
	LMEbound_set(&bounds, &low, &high);
	LMEquadTree_init(&tree, &bounds, 10);

	return 1;
}
