#include <stdlib.h>
#include <stdio.h>
#include <gsl/gsl_math.h>
#include <inttypes.h>
#include "coord.h"
#include "bound.h"
#include "qtree.h"
/**typedef struct LMEquadNode {
	int isEmpty;
	int isLeaf;
	int depth;
	int k; // number of points before subdivision
	LMEbound bounds;
	int length;
	struct LMEquadNode* nw;
	struct LMEquadNode* ne;
	struct LMEquadNode* sw;
	struct LMEquadNode* se;
	struct LMEquadNode* parent;
	uint32_t size;
	LMEpoint *data;
} LMEquadNode;


typedef struct LMEquadTree {
	struct LMEquadNode* root;
	uint64_t size;
	int k; // number of points before subdivision
	LMEbound bounds;
	int length;
	int depth;

} LMEquadTree;
**/
int LMEquadTree_init(LMEquadTree* tree, LMEbound* bounds, int k) {
	LMEbound_copy(&(tree->bounds), bounds);
	tree->k = k;
	tree->size=0;
	tree->length = LMEbound_length(bounds, 0);
	tree->depth = 0;
	LMEquadNode_init(&(tree->root), k, bounds, 0);
	return 1;
			
}

int LMEquadNode_init(LMEquadNode** node, int k, LMEbound* bounds, int depth) {
	LMEquadNode* newNode = (LMEquadNode*)malloc(sizeof(LMEquadNode));
	LMEbound_copy(&(newNode->bounds), bounds);
	newNode->k = k;
	newNode->depth = depth;
	newNode->length = LMEbound_length(bounds, 0);
	newNode->size=0;
	newNode->isEmpty=1;
	newNode->isLeaf=1;
	*node = newNode;
	return 1;
}

int LMEquadTree_getRoot(LMEquadTree* tree, LMEquadNode* root) {
	root = tree->root;	
	return 0;
}

// Get the center point of a quad node, useful for splitting
int LMEquadNode_getCenter(LMEquadNode* node, LMEcoord* centroid) {
	
	*centroid =  LMEbound_Centroid(&node->bounds);
	return 0;
	
}

// Generate child nodes for quadNode and move points to children
int LMEquadNode_split(LMEquadNode* node) {

	return 0;
}

// Add a point to a quadtree, this will add the point to a 
// node based on tree depth, number of points in node and 
// point location
int LMEquadTree_add(LMEquadTree* tree, LMEcoord* coord) {
	// NEED to check which the depth of the tree and 
	// find the leaf which the point fits into
	return 0;
}

