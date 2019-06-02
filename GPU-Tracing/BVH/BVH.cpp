#include "BVH.h"
#include "../shared.h"
#include <limits>

BVH::BVH( std::vector<RTTriangle> &objects, uint32_t leafSize )
	: build_prims( &objects ), leafSize( leafSize ), nNodes( 0 ), nLeafs( 0 ), bvhTree( NULL )
{
	build();
}

BVH::~BVH()
{
	delete[] bvhTree;
}

struct BVHBuildEntry
{
	// If non-zero then this is the index of the parent. (used in offsets)
	uint32_t parent;
	// The range of objects in the object list covered by this node.
	uint32_t start, end;
};

void BVH::build()
{
	BVHBuildEntry todo[128];
	uint32_t stackptr = 0;
	const uint32_t Untouched = 0xffffffff;
	const uint32_t TouchedTwice = 0xfffffffd;

	// Push the root
	todo[stackptr].start = 0;
	todo[stackptr].end = (uint32_t)build_prims->size();
	todo[stackptr].parent = 0xfffffffc;
	stackptr++;

	BVHNode node;
	std::vector<BVHNode> buildnodes;
	buildnodes.reserve( build_prims->size() * 2 );

	while ( stackptr > 0 )
	{
		// Pop the next item off of the stack
		BVHBuildEntry &bnode( todo[--stackptr] );
		uint32_t start = bnode.start;
		uint32_t end = bnode.end;
		uint32_t nPrims = end - start;

		nNodes++;
		node.start = start;
		node.nPrims = nPrims;
		node.rightOffset = Untouched;

		// Calculate the bounding box for this node
		AABB bb = ( *build_prims )[start].getAABB();
		vec3 center = bb.getCentroid();
		AABB bc( center, center );

		for ( uint32_t p = start + 1; p < end; ++p )
		{
			AABB tmpBounds = ( *build_prims )[p].getAABB();
			bb.expandToInclude( tmpBounds );
			bc.expandToInclude( tmpBounds.getCentroid() );
		}

		node.aabb_minaabb_max = bb.aabb_minaabb_max;
		node.aabb_minaabb_min = bb.aabb_minaabb_min;

		// If the number of primitives at this point is less than the leaf
		// size, then this will become a leaf. (Signified by rightOffset == 0)
		if ( nPrims <= leafSize )
		{
			node.rightOffset = 0;
			nLeafs++;
		}

		buildnodes.push_back( node );

		// Child touches parent...
		// Special case: Don't do this for the root.
		if ( bnode.parent != 0xfffffffc )
		{
			buildnodes[bnode.parent].rightOffset--;

			// When this is the second touch, this is the right child.
			// The right child sets up the offset for the flat tree.
			if ( buildnodes[bnode.parent].rightOffset == TouchedTwice )
			{
				buildnodes[bnode.parent].rightOffset = nNodes - 1 - bnode.parent;
			}
		}

		// If this is a leaf, no need to subdivide.
		if ( node.rightOffset == 0 )
			continue;

		// Set the split dimensions
		uint32_t split_dim = bc.maxDimension();
		// Split on the center of the longest axis
		float split_coord = 0.5f * ( bc.aabb_minaabb_min[split_dim] + bc.aabb_minaabb_max[split_dim] );

#ifdef SAH_ON
		getSplitDimAndCoordBySAH( split_dim, split_coord, BIN_NUM, bc, start, end );
#endif

		// Partition the list of objects on this split
		uint32_t mid = start;
		for ( uint32_t i = start; i < end; ++i )
		{
			if ( ( *build_prims )[i].getAABB().getCentroid()[split_dim] < split_coord )
			{
				std::swap( ( *build_prims )[i], ( *build_prims )[mid] );
				++mid;
			}
		}

		// If we get a bad split, just choose the center...
		if ( mid == start || mid == end )
		{
			mid = start + ( end - start ) / 2;
		}

		// Push right child
		todo[stackptr].start = mid;
		todo[stackptr].end = end;
		todo[stackptr].parent = nNodes - 1;
		stackptr++;

		// Push left child
		todo[stackptr].start = start;
		todo[stackptr].end = mid;
		todo[stackptr].parent = nNodes - 1;
		stackptr++;
	}

	// Copy the temp node data to a flat array
	bvhTree = new BVHNode_32[nNodes];
	for ( uint32_t n = 0; n < nNodes; ++n )
	{
		const BVHNode &node = buildnodes[n];

		bvhTree[n].aabb_minaabb_max = node.aabb_minaabb_max;
		bvhTree[n].aabb_minaabb_min = node.aabb_minaabb_min;
		bvhTree[n].count = node.nPrims;
		// leaf
		if (node.rightOffset == 0)
		{
			bvhTree[n].leftFirst = ( node.start << 1 ) + 0;
		}
		else
		{
			bvhTree[n].leftFirst = ( node.rightOffset << 1 ) + 1;
		}
	}
}

uint32_t BVH::getNodesCount()
{
	return nNodes;
}

void BVH::getSplitDimAndCoordBySAH( uint32_t &split_dim, float &split_coord, uint32_t binnedNum, AABB &bc, uint32_t &start, uint32_t &end )
{
	float fCurrentCost = numeric_limits<float>::max();

	for (uint32_t dim=0;dim<3;dim++)
	{
		uint32_t split_dim_current = dim;

		float split_inc_step = ( bc.aabb_minaabb_max[split_dim_current] - bc.aabb_minaabb_min[split_dim_current] ) / binnedNum;
		for ( uint32_t j = 1; j < binnedNum; j++ )
		{
			float split_coord_current = bc.aabb_minaabb_min[split_dim_current] + j * split_inc_step;
			float leftArea = 0;
			float rightArea = 0;

			float leftNumber = 0;
			float rightNumber = 0;

			for ( uint32_t i = start; i < end; ++i )
			{
				const AABB& box = ( *build_prims )[i].getAABB();
				float area = box.surfaceArea();
				/*
				if ( box.aabb_minaabb_min[split_dim_current] < split_coord_current 
                    && box.aabb_minaabb_max[split_dim_current] > split_coord_current )
				{
					float ratio = ( split_coord_current - box.aabb_minaabb_min[split_dim_current] ) / ( box.aabb_minaabb_max[split_dim_current] - box.aabb_minaabb_min[split_dim_current] );
					
					leftArea += area;// * ratio;
					rightArea += area; // * ( 1 - ratio );

					leftNumber++;
					rightNumber++;
				}
				else */
				if ( ( *build_prims )[i].getAABB().getCentroid()[split_dim_current] < split_coord_current )
				{
					leftArea += area;
					leftNumber++;
				}
				else
				{
					rightArea += area;
					rightNumber++;
				}
			}

			float fCost = leftArea * leftNumber + rightArea * rightNumber;
			if ( fCurrentCost > fCost )
			{
				split_dim = split_dim_current;
				split_coord = split_coord_current;

				fCurrentCost = fCost;
			}
		}
	}
}