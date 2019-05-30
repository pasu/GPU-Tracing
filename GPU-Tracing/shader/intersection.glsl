layout( std430, binding = 0 ) buffer SCREEN_BUFFER
{
	vec4 colors[];
};

struct RTRay
{
	vec4 pos;
	vec4 dir;
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

struct wf_PathState
{
	vec3 indirect_pos;
	float pre_pdf_hemi_brdf;

	vec3 indirect_dir;
	float light_weight;

	vec4 final_color; // w: iteration number

	vec3 shadow_pos;
	uint pixelIdx;

	vec3 shadow_dir;
};

layout( std430, binding = 10 ) buffer PathState_BUFFER
{
	wf_PathState ps[];
};

struct wf_queue_counter
{
	uint raygenQueue;
	uint extentionQueue;
	uint shadowQueue;
	uint bump;
};

layout( std430, binding = 11 ) buffer QueueCounter_BUFFER
{
	wf_queue_counter qc;
};

layout( std430, binding = 12 ) buffer genQueue_BUFFER
{
	uint genQueue[];
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