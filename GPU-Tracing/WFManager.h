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