#version 430
layout( local_size_x = 32, local_size_y = 4, local_size_z = 1 ) in;
// SSBO
//////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////
uniform uint frame_id;
uniform mat4 mvMatrix;

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
	ray.dir = normalize( vec4( mvMatrix * vec4( vec3( float( x ) + xorshift32(), float( uint(rp.nWidth) - y ) + xorshift32(), 0.0f ) - vec3( float(rp.nWidth)*0.5f ), 1.0f ) ) - vec4( ray.pos, 1.0f ) ).xyz;
}

void main()
{
	ivec2 storePos = ivec2( gl_GlobalInvocationID.xy );

	uint largePrime1 = 386030683u;
	uint largePrime2 = 919888919u;
	uint largePrime3 = 101414101u;

	random_seed = ( ( uint( storePos.x ) * largePrime1 + uint( storePos.y ) ) * largePrime1 + frame_id * largePrime3 );
	int idx = storePos.y * int( SCRWIDTH ) + storePos.x;
	primaryRay( gl_GlobalInvocationID.x, gl_GlobalInvocationID.y, rays[idx] );
}