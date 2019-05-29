#pragma once
using namespace std;

struct RenderParameters
{
	uint nTaskNum;
	uint nWidth;
	uint nHeight;
	uint nMaxBounces;
    RenderParameters()
    {
		nTaskNum = 1 << 20;
		nWidth = 800u;
		nHeight = 800u;
		nMaxBounces = 8;
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
};

struct wf_queue_counter
{
	uint raygenQueue;
	uint extentionQueue;
	uint shadowQueue;
	uint bump;
};