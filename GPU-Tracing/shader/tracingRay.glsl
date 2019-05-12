#version 430
layout( local_size_x = 32, local_size_y = 4, local_size_z = 1 ) in;
layout( rgba8, binding = 0 ) writeonly uniform highp image2D destTex;

uniform uint frame_id;
uniform mat4 mvMatrix;

struct RTRay
{
	vec4 pos;
	vec4 dir;
};

layout( std430, binding = 0 ) buffer RTRAY_BUFFER
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

layout( std430, binding = 1 ) buffer TRIANGLE_BUFFER
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

layout( std430, binding = 2 ) buffer BVH_BUFFER
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

layout( std430, binding = 3 ) buffer MATERIAL_BUFFER
{
	RTMaterial materials[];
};

layout( std430, binding = 4 ) buffer TEXTURE_BUFFER
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

layout( std430, binding = 5 ) buffer TexInfo_BUFFER
{
	RTTexInfo textureInfos[];
};

uint random_seed;
uint SCRWIDTH = 800u;
uint HALF_SCRWIDTH = 400u;

const float very_large_float = 1e9f;
const float very_small_float = 1e-9f;

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
			return false;
	}

    near = local_tMin;
	far = tMax;

	return true;
}

bool floatEquals(float value, float comp)
{
	return abs( value - comp ) <= very_small_float;
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
	if ( u < 0 || u > 1 )
		return false;

	vec3 Q = cross( T, AB );
	float v = inverseDenominator * dot( Q, dir );
	if ( v < 0 || u + v > 1 )
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
	int i; // Node
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
			continue;

        vec3 hitPnt;
        if ( ( node.leftFirst & 0x1 ) == 0 )
		{
			int start = ( node.leftFirst >> 1 );

            for (int o = 0; o < node.count; ++o)
            {
                if (intersectTriangle(ray, start + o, hitPnt))
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
		return false;

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

void getSurfaceData(RTRay ray, RTIntersection intersection, out SurfaceData hitPnt)
{
	RTTriangle tri = triangles[intersection.triangle_id];

    hitPnt.position = ray.pos.xyz + ray.dir.xyz * intersection.distance;

    hitPnt.normal = normalize( vec3( tri.normals0_0 + intersection.u * ( tri.normals1_0 - tri.normals0_0 ) + intersection.v * ( tri.normals2_0 - tri.normals0_0 ),
									 tri.normals0_1 + intersection.u * ( tri.normals1_1 - tri.normals0_1 ) + intersection.v * ( tri.normals2_1 - tri.normals0_1 ),
									 tri.normals0_2 + intersection.u * ( tri.normals1_2 - tri.normals0_1 ) + intersection.v * ( tri.normals2_2 - tri.normals0_2 ) ) );

    float areaABC, areaPBC, areaPCA;
	Barycentric( hitPnt.position, vec3( tri.vertices0_0, tri.vertices0_1, tri.vertices0_2 ), 
                    vec3( tri.vertices1_0, tri.vertices1_1, tri.vertices1_2 ),
				    vec3( tri.vertices2_0, tri.vertices2_1, tri.vertices2_2 ),
				 areaABC, areaPBC, areaPCA );

    hitPnt.texCoord = vec2( tri.textures0_0, tri.textures0_1 ) * areaABC 
        + vec2( tri.textures1_0, tri.textures1_1 ) * areaPBC 
        + vec2( tri.textures2_0, tri.textures2_1 ) * areaPCA;

    hitPnt.materialID = tri.mIndex;

    return;
}

vec3 getTexelFromFile(RTTexInfo info, int x, int y)
{
	int basePixel = info.offset / 4 + ( x + ( ( ( info.height - 1 ) - y ) * info.width ) );
	return vec3( texture_buf[basePixel * 3], texture_buf[basePixel * 3 + 1], texture_buf[basePixel * 3 + 2] );
}

vec3 bilinearInterpolation(RTTexInfo info, float u, float v)
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

    return ( 1.0f - uPrime ) * ( 1.0f - vPrime ) * getTexelFromFile( info, xl, yb) +
		   uPrime * ( 1.0f - vPrime ) * getTexelFromFile( info, xr, yb ) +
		   ( 1.0f - uPrime ) * vPrime * getTexelFromFile( info, xl, yt ) +
		   uPrime * vPrime * getTexelFromFile( info, xr, yt);
}

vec3 getTexel( RTTexInfo info, float s, float t )
{
	vec2 scale = vec2( 1.0f );

	s *= scale.x;
	t *= scale.y;

	float wrappedS = s - floor(s);
	float wrappedT = t - floor( t );

    return bilinearInterpolation( info, wrappedS, wrappedT );
}

vec4 getAlbedoAtPoint( SurfaceData hitPnt )
{
	RTMaterial material = materials[hitPnt.materialID];

    if (floatEquals(texture_buf[3], 0.1f))
    {
		return vec4( 1, 1, 0, 1 );
    }

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

void main()
{
	ivec2 storePos = ivec2( gl_GlobalInvocationID.xy );

    int idx = storePos.y * int( SCRWIDTH ) + storePos.x;
	RTRay ray = rays[idx];

    float c = 1.0;
	vec4 col = vec4( c * 0.83, c, min( c * 1.3, 1.0 ), 1 );
	/*
    vec3 aabb_min = vec3( -100, -100, -900 );
	vec3 aabb_max = vec3( 100, 100, -800 );
	if ( intersectAABB( aabb_min, aabb_max, ray ) )
	{
		col = vec4( 1, 0, 0, 1 );
	}
	*/

	RTIntersection intersection;
    if (getIntersection(ray, intersection))
    {
		SurfaceData hitPnt;
		getSurfaceData( ray, intersection, hitPnt );
		col = getAlbedoAtPoint( hitPnt );
    }

	/*
    for ( int i = 0; i < count; i++ )
	{
		if ( intersectTriangle( ray, i, hitPnt ) )
		{
			col = vec4( 1, 0, 0, 1 );
			break;
		}
	}
    */
    
	imageStore( destTex, storePos, col );
}