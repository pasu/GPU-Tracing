const wf_extension = `#version 310 es
layout( local_size_x = 128, local_size_y = 1, local_size_z = 1 ) in;

//All SSBO should copy from ssbo.glsl
///////////////////////////////////////////////
layout( std430, binding = 0 ) buffer SCREEN_BUFFER
{
	vec4 colors[];
};

struct RTRay
{
	vec3 pos;
	float pre_pdf_hemi_brdf;

	vec3 dir;
	uint pixelIdx;

	vec3 shadow_dir;
	int hit_triangle_id;

	vec3 color_obj;
	float hit_u;

	vec3 brdf_weight;
	int hit_materialID;

	vec3 final_color;
	float hit_v;

	vec3 hit_position;
	float hit_distance;

	vec3 hit_normal;
	int shadowRayBlocked;

	vec3 finalColor;
	uint bounceNum;

	vec4 albedo;

	vec2 hit_texCoord;
	uint bContinue;
	float pdf_hemi_brdf;

	vec3 random_dir;
	float brdf;

	vec3 light_color;
};

layout( std430, binding = 1 ) buffer RTRAY_BUFFER
{
	RTRay rays[];
};

struct RTTriangle
{
	float vertices0_0;
	float vertices0_1;
	float vertices0_2;

	float vertices1_0;
	float vertices1_1;
	float vertices1_2;

	float vertices2_0;
	float vertices2_1;
	float vertices2_2;

	float normals0_0;
	float normals0_1;
	float normals0_2;

	float normals1_0;
	float normals1_1;
	float normals1_2;

	float normals2_0;
	float normals2_1;
	float normals2_2;

	float textures0_0;
	float textures0_1;

	float textures1_0;
	float textures1_1;

	float textures2_0;
	float textures2_1;

	int mIndex;
};

layout( std430, binding = 2 ) buffer TRIANGLE_BUFFER
{
	RTTriangle triangles[];
};

struct BVHNode_32
{
	vec3 aabb_minaabb_min;
	int leftFirst;
	vec3 aabb_minaabb_max;
	int count;
};

layout( std430, binding = 3 ) buffer BVH_BUFFER
{
	BVHNode_32 bvh_nodes[];
};

struct RTMaterial
{
	vec3 color;
	float reflectionFactor;

	vec3 emission;
	float indexOfRefraction;

	int shadingType;
	int texID;
	float m_pow;
	float m_k;
};

layout( std430, binding = 4 ) buffer MATERIAL_BUFFER
{
	RTMaterial materials[];
};

layout( std430, binding = 5 ) buffer TEXTURE_BUFFER
{
	float texture_buf[];
};

struct RTTexInfo
{
	int idx;
	int offset;
	int width;
	int height;
};

layout( std430, binding = 6 ) buffer TexInfo_BUFFER
{
	RTTexInfo textureInfos[];
};

struct RTLightBoundary
{
	vec4 v1;
	vec4 v2;
	vec4 v3;
	vec4 v4;
	vec4 normal;
};

layout( std430, binding = 7 ) buffer LIGHT_BUFFER
{
	RTLightBoundary lights[];
};

layout( std430, binding = 8 ) buffer LIGHT_NUM_BUFFER
{
	int light_num;
};

struct RenderParameters
{
	uint nTaskNum;
	uint nWidth;
	uint nHeight;
	uint nMaxBounces;
};

layout( std430, binding = 9 ) buffer RenderParameters_BUFFER
{
	RenderParameters rp;
};

struct wf_queue_counter
{
	uint raygenQueue;
	uint extensionQueue;
	uint shadowQueue;
	uint materialQueue;
};

layout( std430, binding = 10 ) buffer QueueCounter_BUFFER
{
	wf_queue_counter qc;
};

layout( std430, binding = 11 ) buffer genQueue_BUFFER
{
	uint rayGenQueue[];
};

layout( std430, binding = 12 ) buffer materialQueue_BUFFER
{
	uint materialQueue[];
};

layout( std430, binding = 13 ) buffer ExtensionQueue_BUFFER
{
	uint extensionQueue[];
};

layout( std430, binding = 14 ) buffer ShadowQueue_BUFFER
{
	uint shadowQueue[];
};
///////////////////////////////////////////////
const float very_large_float = 1e9f;
const float very_small_float = 1e-9f;

bool floatEquals( float value, float comp )
{
	return abs( value - comp ) <= very_small_float;
}

bool intersectAABB( vec3 aabb_min, vec3 aabb_max, RTRay ray, out float near, out float far )
{
	vec3 origin = ray.pos.xyz;
	vec3 dir = ray.dir.xyz;
	float local_tMin = 0.001;
	float tMax = very_large_float;
	for ( int i = 0; i < 3; i++ )
	{
		float invD = 1.0f / dir[i];
		float t0 = ( aabb_min[i] - origin[i] ) * invD;
		float t1 = ( aabb_max[i] - origin[i] ) * invD;
		if ( invD < 0.0f )
		{
			float tmp = t0;
			t0 = t1;
			t1 = tmp;
		}

		local_tMin = max( t0, local_tMin );
		tMax = min( t1, tMax );
		if ( tMax < local_tMin )
		{
			return false;
		}
	}

	near = local_tMin;
	far = tMax;

	return true;
}

bool intersectTriangle( RTRay ray, RTTriangle t, out vec3 pnt )
{
	pnt = vec3( -1, -1, -1 );

	vec3 origin = ray.pos.xyz;
	vec3 dir = ray.dir.xyz;

	vec3 a = vec3( t.vertices0_0, t.vertices0_1, t.vertices0_2 );
	vec3 b = vec3( t.vertices1_0, t.vertices1_1, t.vertices1_2 );
	vec3 c = vec3( t.vertices2_0, t.vertices2_1, t.vertices2_2 );

	vec3 AB = b - a;
	vec3 AC = c - a;

	vec3 P = cross( dir, AC );
	float denominator = dot( P, AB ); //if negative triangle is backfacing (Cull here)

	if ( floatEquals( denominator, 0.0f ) ) //ray parallel to triangle
		return false;

	float inverseDenominator = 1.0f / denominator;
	vec3 T = origin - a;
	float u = inverseDenominator * dot( P, T );
	if ( u < 0.0f || u > 1.0f )
		return false;

	vec3 Q = cross( T, AB );
	float v = inverseDenominator * dot( Q, dir );
	if ( v < 0.0f || u + v > 1.0f )
		return false;

	pnt.x = u;
	pnt.y = v;
	pnt.z = inverseDenominator * dot( Q, AC );

	return true;
}

bool intersectTriangle( RTRay ray, int idx, out vec3 pnt )
{
	RTTriangle t = triangles[idx];

	return intersectTriangle( ray, t, pnt );
}

struct BVHTraversal
{
	int i;		// Node
	float mint; // Minimum hit time for this node.
};

struct RTIntersection
{
	float u;
	float v;
	float distance;
	int triangle_id;
};

bool getIntersection( RTRay ray, out RTIntersection intersection )
{
	intersection.triangle_id = -1;
	intersection.distance = -1.0f;

	float bbhits[4];
	int closer, other;

	// Working set
	BVHTraversal todo[256];
	int stackptr = 0;

	// "Push" on the root node to the working set
	todo[stackptr].i = 0;
	todo[stackptr].mint = -9999999.f;

	while ( stackptr >= 0 )
	{
		// Pop off the next node to work on.
		int ni = todo[stackptr].i;
		float fnear = todo[stackptr].mint;
		stackptr--;
		BVHNode_32 node = bvh_nodes[ni];

		// If this node is further than the closest found intersection, continue
		if ( intersection.distance > 0.0f && fnear > intersection.distance )
		{
			continue;
		}

		vec3 hitPnt;
		if ( ( node.leftFirst & 0x1 ) == 0 )
		{
			int start = ( node.leftFirst >> 1 );

			for ( int o = 0; o < node.count; ++o )
			{
				if ( intersectTriangle( ray, start + o, hitPnt ) )
				{
					if ( intersection.distance < 0.0f || hitPnt.z < intersection.distance )
					{
						intersection.u = hitPnt.x;
						intersection.v = hitPnt.y;
						intersection.distance = hitPnt.z;
						intersection.triangle_id = start + o;
					}
				}
			}
		}
		else
		{
			int rightOffset = ( node.leftFirst >> 1 );
			bool hitc0 = intersectAABB( bvh_nodes[ni + 1].aabb_minaabb_min, bvh_nodes[ni + 1].aabb_minaabb_max, ray,
										bbhits[0], bbhits[1] );
			bool hitc1 = intersectAABB( bvh_nodes[ni + rightOffset].aabb_minaabb_min,
										bvh_nodes[ni + rightOffset].aabb_minaabb_max, ray,
										bbhits[2], bbhits[3] );

			if ( hitc0 && hitc1 )
			{

				// We assume that the left child is a closer hit...
				closer = ni + 1;
				other = ni + rightOffset;

				if ( bbhits[2] < bbhits[0] )
				{
					float tmp = bbhits[0];
					bbhits[0] = bbhits[2];
					bbhits[2] = tmp;

					tmp = bbhits[1];
					bbhits[1] = bbhits[3];
					bbhits[3] = tmp;

					int id = closer;
					closer = other;
					other = id;
				}

				// It's possible that the nearest object is still in the other side, but we'll
				// check the further-awar node later...

				// Push the farther first
				BVHTraversal bvhT1;
				bvhT1.i = other;
				bvhT1.mint = bbhits[2];
				todo[++stackptr] = bvhT1;

				// And now the closer (with overlap test)
				BVHTraversal bvhT2;
				bvhT2.i = closer;
				bvhT2.mint = bbhits[0];
				todo[++stackptr] = bvhT2;
			}
			else if ( hitc0 )
			{
				BVHTraversal bvhT;
				bvhT.i = ni + 1;
				bvhT.mint = bbhits[0];
				todo[++stackptr] = bvhT;
			}
			else if ( hitc1 )
			{
				BVHTraversal bvhT;
				bvhT.i = ni + rightOffset;
				bvhT.mint = bbhits[2];
				todo[++stackptr] = bvhT;
			}
		}
	}

	if ( intersection.triangle_id == -1 )
	{
		return false;
	}
	return true;
}

struct SurfaceData
{
	vec3 position;
	vec3 normal;
	vec2 texCoord;
	int materialID;
};

void Barycentric( vec3 p, vec3 a, vec3 b, vec3 c, out float u, out float v, out float w )
{
	vec3 v0 = b - a, v1 = c - a, v2 = p - a;
	float d00 = dot( v0, v0 );
	float d01 = dot( v0, v1 );
	float d11 = dot( v1, v1 );
	float d20 = dot( v2, v0 );
	float d21 = dot( v2, v1 );
	float denom = d00 * d11 - d01 * d01;
	v = ( d11 * d20 - d01 * d21 ) / denom;
	w = ( d00 * d21 - d01 * d20 ) / denom;
	u = 1.0f - v - w;
}

void getSurfaceData( RTRay ray, RTIntersection intersection, out SurfaceData hitPnt )
{
	RTTriangle tri = triangles[intersection.triangle_id];

	hitPnt.position = ray.pos.xyz + ray.dir.xyz * intersection.distance;

	vec3 tv1 = vec3( tri.vertices0_0, tri.vertices0_1, tri.vertices0_2 );
	vec3 tv2 = vec3( tri.vertices1_0, tri.vertices1_1, tri.vertices1_2 );
	vec3 tv3 = vec3( tri.vertices2_0, tri.vertices2_1, tri.vertices2_2 );
	/*
    vec3 dir_x = tv2 - tv1;
    vec3 dir_y = tv3 - tv1;
    
      
    vec3 normalR = cross( dir_x, dir_y );
    hitPnt.normal = normalize( normalR );
    */

	hitPnt.normal = normalize( vec3( tri.normals0_0 + intersection.u * ( tri.normals1_0 - tri.normals0_0 ) + intersection.v * ( tri.normals2_0 - tri.normals0_0 ),
									 tri.normals0_1 + intersection.u * ( tri.normals1_1 - tri.normals0_1 ) + intersection.v * ( tri.normals2_1 - tri.normals0_1 ),
									 tri.normals0_2 + intersection.u * ( tri.normals1_2 - tri.normals0_1 ) + intersection.v * ( tri.normals2_2 - tri.normals0_2 ) ) );

	float areaABC, areaPBC, areaPCA;
	Barycentric( hitPnt.position, tv1, tv2, tv3,
				 areaABC, areaPBC, areaPCA );

	hitPnt.texCoord = vec2( tri.textures0_0, tri.textures0_1 ) * areaABC + vec2( tri.textures1_0, tri.textures1_1 ) * areaPBC + vec2( tri.textures2_0, tri.textures2_1 ) * areaPCA;

	hitPnt.materialID = tri.mIndex;

	return;
}

bool isLight( RTMaterial material )
{
	if ( floatEquals( material.emission.x, 0.0f ) && floatEquals( material.emission.y, 0.0f ) && floatEquals( material.emission.z, 0.0f ) )
	{
		return false;
	}

	return true;
}

void main()
{
	uint globalIdx = gl_GlobalInvocationID.x;
	if ( globalIdx > qc.extensionQueue )
		return;

    uint gid = extensionQueue[globalIdx];

    RTIntersection intersection;

    RTRay ray;
	ray.pos = rays[gid].pos;
	ray.dir = rays[gid].dir;

	bool bHit = getIntersection( ray, intersection );

    rays[gid].hit_triangle_id = -1;

    if (bHit)
    {
		SurfaceData hitPnt;
		getSurfaceData( ray, intersection, hitPnt );

		rays[gid].hit_triangle_id = intersection.triangle_id;
		rays[gid].hit_u = intersection.u;
		rays[gid].hit_v = intersection.v;
		rays[gid].hit_distance = intersection.distance;

        rays[gid].hit_position = hitPnt.position;
		rays[gid].hit_normal = hitPnt.normal;
		rays[gid].hit_texCoord = hitPnt.texCoord;
		rays[gid].hit_materialID = hitPnt.materialID;

        RTMaterial material = materials[hitPnt.materialID];

		// light material, MIS
		if ( !isLight( material ) )
		{
			uint materialIdx = atomicAdd( qc.materialQueue, 1u );
			materialQueue[materialIdx] = gid;

            uint shadowIdx = atomicAdd( qc.shadowQueue, 1u );
			shadowQueue[shadowIdx] = gid;
		}
    }

    return;
}
`;