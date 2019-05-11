#pragma once
#include "../DS/datastructure.h"
#include <vector>

using namespace std;

struct BVHNode_32
{
	//merge start and rightOffset as leftFirst;
	vec3 aabb_minaabb_min;
	int leftFirst;
	vec3 aabb_minaabb_max;
	int count;
};

struct BVHNode
{
	vec3 aabb_minaabb_min;
	vec3 aabb_minaabb_max;
	uint32_t start, nPrims, rightOffset;
	//uint32_t leftFirst, count;
};

class BVH
{
  public:
	BVH( std::vector<RTTriangle>& objects, uint32_t leafSize = 4 );
	~BVH();

	std::vector<RTTriangle> *build_prims;

	//! Build the BVH tree out of build_prims
	void build();

    uint32_t getNodesCount();

	// Fast Traversal System
	BVHNode_32 *bvhTree;

	void getSplitDimAndCoordBySAH( uint32_t &split_dim, float &split_coord, uint32_t binnedNum, AABB &bc, uint32_t &start, uint32_t &end );

  private:
	uint32_t nNodes, nLeafs, leafSize;
};
