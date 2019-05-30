#pragma once
using namespace std;

struct RenderParameters
{
	uint nTaskNum;
	uint nWidth;
	uint nHeight;
	uint nMaxBounces;

    vec4 color_scene;

    RenderParameters()
    {
		nTaskNum = 1 << 20;
		nWidth = 800u;
		nHeight = 800u;
		nMaxBounces = 5;

        color_scene = vec4(1.0,0,0,1.0);
    }
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
	int hit_triangle_id;

	vec3 color_obj;
	float hit_u;

	vec3 brdf_weight;
	float hit_v;

	float hit_distance;
	int shadowRayBlocked;
};

struct wf_queue_counter
{
	uint raygenQueue;
	uint extensionQueue;
	uint shadowQueue;
	uint materialQueue;

    wf_queue_counter()
    {
		raygenQueue = 0u;
		extensionQueue = 0u;
		shadowQueue = 0u;
		materialQueue = 0u;
    }
};