const wf_rayGen = `#version 310 es
layout( local_size_x = 128, local_size_y = 1, local_size_z = 1 ) in;

uniform uint frame_id;
uniform uint total_num;
uniform mat4 mvMatrix;

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

struct RenderParameters
{
	uint nTaskNum;
	uint nWidth;
	uint nHeight;
	uint nMaxBounces;

	vec4 color_scene;
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

layout( std430, binding = 14 ) buffer ExtensionQueue_BUFFER
{
	uint extensionQueue[];
};
////////////////////////////////////////////////////////////////////

uint random_seed;

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
	ray.dir = normalize( vec4( mvMatrix * vec4( vec3( float( x ) + xorshift32(), float( uint(rp.nWidth) - y ) + xorshift32(), 0.0f ) - vec3( float(rp.nWidth)*0.5f ), 1.0f ) ) - vec4( ray.pos, 1.0f ) ).xyz;
}

void main()
{
	uint globalIdx = gl_GlobalInvocationID.x;

    if ( globalIdx > qc.raygenQueue )
		return;

	uint gid = rayGenQueue[globalIdx];
	uint curIdx = globalIdx + total_num;
	curIdx = curIdx % ( rp.nWidth * rp.nHeight );

	uint xId = curIdx % rp.nWidth;
	uint yId = curIdx / rp.nWidth;

	uint largePrime1 = 386030683u;
	uint largePrime2 = 919888919u;
	uint largePrime3 = 101414101u;

	random_seed = ( ( xId * largePrime1 + yId ) * largePrime1 + frame_id * largePrime3 );

	primaryRay( xId, yId, rays[gid] );
    rays[gid].pixelIdx = curIdx;

    rays[gid].finalColor = vec3( 0 );
	rays[gid].color_obj = vec3( 1 );
	rays[gid].brdf_weight = vec3( 1.0f );
	rays[gid].pre_pdf_hemi_brdf = 1.0f;
	rays[gid].bounceNum = 0u;
	rays[gid].albedo = vec4( 0 );

    uint extIdx = atomicAdd( qc.extensionQueue, 1u );
	extensionQueue[extIdx] = gid;
}
`;