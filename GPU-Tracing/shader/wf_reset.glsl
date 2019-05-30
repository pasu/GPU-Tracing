#version 430
layout( local_size_x = 128, local_size_y = 1, local_size_z = 1 ) in;

//All SSBO should copy from ssbo.glsl
///////////////////////////////////////////////
layout( std430, binding = 0 ) buffer SCREEN_BUFFER
{
	vec4 colors[];
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
///////////////////////////////////////////////

uniform uint frame_id;

void main()
{
	uint gid = gl_GlobalInvocationID.x;
	if ( gid > rp.nTaskNum )
		return;
	
    if ( gid < rp.nWidth * rp.nHeight && frame_id == 0 )
    {
		colors[gid] = vec4( 0.0f );
    }
    
    if (gid == 0)
    {
		qc.raygenQueue = rp.nTaskNum;
    }

    rayGenQueue[gid] = gid;
}