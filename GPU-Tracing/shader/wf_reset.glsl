#version 430
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

uniform uint frame_id;

void main()
{
	uint gid = gl_GlobalInvocationID.x;
	if ( gid > rp.nTaskNum )
		return;
	
    if ( gid < rp.nWidth * rp.nHeight && frame_id == 0u )
    {
		colors[gid] = vec4( 0.0f );
    }
    
    if (gid == 0u)
    {
		qc.raygenQueue = rp.nTaskNum;
    }

    rayGenQueue[gid] = gid;
}