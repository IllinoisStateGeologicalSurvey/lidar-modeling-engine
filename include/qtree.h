#include <stdlib.h>
#include <stdio.h>
#include <gsl/gsl_math.h>
#include <inttypes.h>
#include "coord.h"
#include "bound.h"

typedef struct LMEquadNode {
	int isEmpty;
	int isLeaf;
	int depth;
	int k;
	double length;
	LMEbound bounds;
	uint32_t size;
	struct LMEquadNode* nw;
	struct LMEquadNode* ne;
	struct LMEquadNode* sw;
	struct LMEquadNode* se;
	LMEpoint* data;
} LMEquadNode;

typedef struct LMEquadTree {
	struct LMEquadNode* root;
	uint64_t size;
	int k;
	LMEbound bounds;
	int length;
	int depth;
} LMEquadTree;

int LMEquadTree_init(LMEquadTree* tree, LMEbound* bounds, int k);

int LMEquadNode_init(LMEquadNode** node, int k, LMEbound* bounds, int depth);

int LMEquadTree_getRoot(LMEquadTree* tree, LMEquadNode* root);

int LMEquadNode_getCenter(LMEquadNode * node, LMEcoord* centroid);

int LMEquadNode_split(LMEquadNode* node);

int LMEquadTree_add(LMEquadTree* tree, LMEcoord* coord);


