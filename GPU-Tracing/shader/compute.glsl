#version 430
layout( local_size_x = 32, local_size_y = 4, local_size_z = 1 ) in;
layout( rgba8, binding = 0 ) writeonly uniform highp image2D destTex;

uniform uint frame_id;
uniform mat4 mvMatrix;

struct RTRay
{
	vec3 pos;
	vec3 dir;
};

uint random_seed;
uint SCRWIDTH = 800u;
uint HALF_SCRWIDTH = 400u;

const float very_large_float = 1e9f;

float xorshift32()
{
	random_seed ^= random_seed << 13;
	random_seed ^= random_seed >> 17;
	random_seed ^= random_seed << 5;
	return float( random_seed ) * 2.3283064365387e-10f;
}

vec4 randomDirection()
{
	float longitude = xorshift32() * 3.1415926535897932384626433832795f * 2.0f;
	float lattitude = xorshift32() * 3.1415926535897932384626433832795f;
	float z = sin( lattitude );
	float s = cos( lattitude );
	return vec4( s * cos( longitude ), s * sin( longitude ), z, z );
}

void primaryRay( in uint x, in uint y, out RTRay ray )
{
	ray.pos = vec4( mvMatrix * vec4( 0, 0, 0, 1 ) ).xyz;
	ray.dir = normalize( vec4( mvMatrix * vec4( vec3( float( x ) + xorshift32(), float( SCRWIDTH - y ) + xorshift32(), 0 ) - vec3( HALF_SCRWIDTH ), 1 ) ).xyz - ray.pos );
}

bool intersectAABB( vec3 aabb_min, vec3 aabb_max, RTRay ray )
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
			return false;
	}
	return true;
}

void main()
{
	ivec2 storePos = ivec2( gl_GlobalInvocationID.xy );

	uint largePrime1 = 386030683u;
	uint largePrime2 = 919888919u;
	uint largePrime3 = 101414101u;

	random_seed = ( ( uint( storePos.x ) * largePrime1 + uint( storePos.y ) ) * largePrime1 + frame_id * largePrime3 );

	RTRay ray;
	primaryRay( gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, ray );

	float c = 1.0;
	vec4 col = vec4( c * 0.83, c, min( c * 1.3, 1.0 ), 1 );

	vec3 aabb_min = vec3( -100, -100, -900 );
	vec3 aabb_max = vec3( 100, 100, -800 );
	if ( intersectAABB( aabb_min, aabb_max, ray ) )
	{
		col = vec4( 1, 0, 0, 1 );
	}

	imageStore( destTex, storePos, col );
}