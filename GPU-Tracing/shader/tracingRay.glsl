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
};

layout( std430, binding = 1 ) buffer TRIANGLE_BUFFER
{
	RTTriangle triangles[];
};

layout( std430, binding = 2 ) buffer TRIANGLE_NUMBER
{
	int count;
};

uint random_seed;
uint SCRWIDTH = 800u;
uint HALF_SCRWIDTH = 400u;

const float very_large_float = 1e9f;
const float very_small_float = 1e-9f;

bool intersectAABB( vec3 aabb_min, vec3 aabb_max, RTRay ray )
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
	return true;
}

bool floatEquals(float value, float comp)
{
	return abs( value - comp ) <= very_small_float;
}

vec3 intersectTriangle( RTRay ray, RTTriangle t )
{
	vec3 res = vec3( -1, -1, -1 );
    
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
		return res;

	float inverseDenominator = 1.0f / denominator;
	vec3 T = origin - a;
	float u = inverseDenominator * dot( P, T );
	if ( u < 0 || u > 1 )
		return res;

	vec3 Q = cross( T, AB );
	float v = inverseDenominator * dot( Q, dir );
	if ( v < 0 || u + v > 1 )
		return res;

	res.x = u;
	res.y = v;
	res.z = inverseDenominator * dot( Q, AC );

	return res;
}

vec3 intersectTriangle( RTRay ray, int idx )
{
	RTTriangle t = triangles[idx];

	return intersectTriangle( ray, t );
}

vec3 intersectTriangle_( RTRay r, int idx )
{
	RTTriangle t = triangles[idx];

    vec3 a = vec3( t.vertices0_0, t.vertices0_1, t.vertices0_2 );
	vec3 b = vec3( t.vertices1_0, t.vertices1_1, t.vertices1_2 );
	vec3 c = vec3( t.vertices2_0, t.vertices2_1, t.vertices2_2 );

    vec3 origin = r.pos.xyz;
	vec3 dir = r.dir.xyz;

	mat3 A = mat3( a.xyz - b.xyz, a.xyz - c.xyz, dir );

	int d = int( abs( determinant( A ) ) < very_small_float );

	vec3 l = inverse( A ) * ( a.xyz - origin );

	l.z = ( int( l.x < 0 || l.y < 0 || ( ( l.x + l.y ) > 1 ) || l.z < 0 ) + d ) * very_large_float + l.z * ( 1 - d );

	return l;
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
    for ( int i = 0; i < count; i++ )
	{
		vec3 res = intersectTriangle( ray, i );
		if ( !floatEquals( res.x, -1.0f ) )
		{
			col = vec4( 1, 0, 0, 1 );
			break;
		}
	}
    
	imageStore( destTex, storePos, col );
}