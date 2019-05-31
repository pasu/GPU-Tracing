const wf_material = `#version 310 es
layout( local_size_x = 128, local_size_y = 1, local_size_z = 1 ) in;
layout( rgba8, binding = 0 ) writeonly uniform highp image2D destTex;

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

layout( std430, binding = 11 ) buffer QueueCounter_BUFFER
{
	wf_queue_counter qc;
};

layout( std430, binding = 12 ) buffer genQueue_BUFFER
{
	uint rayGenQueue[];
};

layout( std430, binding = 13 ) buffer materialQueue_BUFFER
{
	uint materialQueue[];
};

layout( std430, binding = 14 ) buffer ExtensionQueue_BUFFER
{
	uint extensionQueue[];
};

layout( std430, binding = 15 ) buffer ShadowQueue_BUFFER
{
	uint shadowQueue[];
};
///////////////////////////////////////////////

uint random_seed;
uint SCRWIDTH = 800u;
uint HALF_SCRWIDTH = 400u;

const float very_large_float = 1e9f;
const float very_small_float = 1e-9f;
const uint SAMPLE_NUM = 16u * 16u;

const vec4 color_scene = vec4( 0.13f, 1, 1, 0 );

float xorshift32()
{
	random_seed ^= random_seed << 13;
	random_seed ^= random_seed >> 17;
	random_seed ^= random_seed << 5;
	return float( random_seed ) * 2.3283064365387e-10f;
}

bool floatEquals( float value, float comp )
{
	return abs( value - comp ) <= very_small_float;
}

bool isLight( RTMaterial material )
{
	if ( floatEquals( material.emission.x, 0.0f ) && floatEquals( material.emission.y, 0.0f ) && floatEquals( material.emission.z, 0.0f ) )
	{
		return false;
	}

	return true;
}

bool intersectAABB( vec3 aabb_min, vec3 aabb_max, RTRay ray, out float near, out float far )
{
	vec3 origin = ray.pos;
	vec3 dir = ray.dir;
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

	vec3 origin = ray.pos;
	vec3 dir = ray.dir;

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

	hitPnt.position = ray.pos + ray.dir * intersection.distance;

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

vec3 getTexelFromFile( RTTexInfo info, int x, int y )
{
	int offset = info.offset / 4;
	int basePixel = ( x + ( ( ( info.height - 1 ) - y ) * info.width ) );
	return vec3( texture_buf[offset + basePixel * 3], texture_buf[offset + basePixel * 3 + 1], texture_buf[offset + basePixel * 3 + 2] );
}

vec3 bilinearInterpolation( RTTexInfo info, float u, float v )
{
	int width = info.width;
	int height = info.height;
	float pu = float( width - 1 ) * u;
	float pv = float( height - 1 ) * v;
	int x = int( pu );
	int y = int( pv );
	float uPrime = pu - float( x );
	float vPrime = pv - float( y );

	int xl = x - 1 >= 0 ? x - 1 : width - 1;
	int xr = x + 1 < width ? x + 1 : 0;
	int yb = y - 1 >= 0 ? y - 1 : height - 1;
	int yt = y + 1 < height ? y + 1 : 0;

	return ( 1.0f - uPrime ) * ( 1.0f - vPrime ) * getTexelFromFile( info, xl, yb ) +
		   uPrime * ( 1.0f - vPrime ) * getTexelFromFile( info, xr, yb ) +
		   ( 1.0f - uPrime ) * vPrime * getTexelFromFile( info, xl, yt ) +
		   uPrime * vPrime * getTexelFromFile( info, xr, yt );
}

vec3 getTexel( RTTexInfo info, float s, float t )
{
	vec2 scale = vec2( 1.0f );

	s *= scale.x;
	t *= scale.y;

	float wrappedS = s - floor( s );
	float wrappedT = t - floor( t );

	return bilinearInterpolation( info, wrappedS, wrappedT );
}

vec4 getAlbedoAtPoint( SurfaceData hitPnt )
{
	RTMaterial material = materials[hitPnt.materialID];

	if ( material.texID == -1 )
	{
		return vec4( material.color, 1.0 );
	}
	else
	{
		int texInfoID = material.texID;
		return vec4( getTexel( textureInfos[texInfoID], hitPnt.texCoord.x, hitPnt.texCoord.y ), 1.0f );
	}
}

const float RT_PI = 3.1415926535897f;
const float INV_PI = 1.0f / RT_PI;
const float shadowBias = 3.1f;

vec3 sampleCosHemisphere( vec3 normal )
{
	vec3 u, v, w;
	w = normal;

	if ( abs( w.x ) > abs( w.y ) )
		u = normalize( cross( vec3( 0, 1, 0 ), w ) );
	else
		u = normalize( cross( vec3( 1, 0, 0 ), w ) );

	v = cross( w, u );

	float a = xorshift32(), b = xorshift32();
	float sini = sqrt( a ), cosi = 2.0f * RT_PI * b;

	return normalize( ( sini * cos( cosi ) * u ) + ( sini * sin( cosi ) * v ) + ( sqrt( 1.0f - a ) * w ) );
}

vec3 random_in_unit_sphere()
{
	vec3 vec;

	do
	{
		vec = 2.0f * vec3( xorshift32(), xorshift32(), xorshift32() ) - vec3( 1.0f, 1.0f, 1.0f );
	} while ( length( vec ) >= 1.0f );

	return normalize( vec );
}

void sampleDiffuse( vec3 normal, out vec3 in_dir, out float pdf )
{
	in_dir = sampleCosHemisphere( normal );
	pdf = dot( normal, in_dir ) * INV_PI;
}

bool evaluate( RTMaterial material, RTRay ray, SurfaceData hitPnt,
			   out vec3 random_dir, out float pdf, out vec4 albedo )
{
	albedo = getAlbedoAtPoint( hitPnt );

	switch ( material.shadingType )
	{
	case 1:
		sampleDiffuse( hitPnt.normal, random_dir, pdf );
		return true;
		break;
	default:
		break;
	}

	return false;
}

float material_brdf( RTMaterial material, RTRay ray, SurfaceData hitPnt, RTRay ray_random )
{
	float cosine = 0.0f;
	vec3 reflect_dir;

	switch ( material.shadingType )
	{
	case 1:
		cosine = dot( hitPnt.normal, ray_random.dir );
		cosine = cosine < 0.0 ? 0.0f : cosine;
		return cosine * INV_PI;
		break;
	default:
		break;
	}
}

void getRandomPnt( out vec3 pnt, out vec3 normal, out float area )
{
	//vec2 uv = vec2( xorshift32(), xorshift32() );
	vec2 uv = vec2( 0.5f );

	vec3 x_dir = lights[0].v2.xyz - lights[0].v1.xyz;
	vec3 y_dir = lights[0].v4.xyz - lights[0].v1.xyz;

	pnt = lights[0].v1.xyz + x_dir * uv.x + y_dir * uv.y;
	normal = lights[0].normal.xyz;

	area = length( x_dir ) * length( y_dir );
	return;
}

float BalanceHeuristicWeight( float pdf1, float pdf2 )
{
	return ( pdf1 ) / ( pdf1 + pdf2 );
}

const int DIFFUSE = 1;
const int maxRecursionDepth = 5;


void main()
{
	uint globalIdx = gl_GlobalInvocationID.x;

	if ( globalIdx > qc.materialQueue )
		return;

	uint gid = materialQueue[globalIdx];

    SurfaceData hitPnt;
	hitPnt.position = rays[gid].hit_position;
	hitPnt.normal = rays[gid].hit_normal;
	hitPnt.texCoord = rays[gid].hit_texCoord;
	hitPnt.materialID = rays[gid].hit_materialID;

    RTMaterial material = materials[hitPnt.materialID];

    bool bContinue = evaluate( material, rays[gid], hitPnt, rays[gid].random_dir, rays[gid].pdf_hemi_brdf, rays[gid].albedo );
	rays[gid].bContinue = uint( bContinue );

    RTRay ray_random;
	ray_random.pos = hitPnt.position + shadowBias * rays[gid].random_dir;
	ray_random.dir = rays[gid].random_dir;
	rays[gid].brdf = material_brdf( material, rays[gid], hitPnt, ray_random );

}
`;