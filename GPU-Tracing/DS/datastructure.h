#pragma once

#include "../pch.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;

struct RTRay
{
	vec4 pos;
	vec4 dir;
};

struct RTTriangle
{
	vec3 vertices[3];
	vec3 normals[3];
	vec2 textures[3];

	RTTriangle( const vec3 &v1, const vec3 &v2, const vec3 &v3, const vec3 &n1, const vec3 &n2, const vec3 &n3, const vec2 &t1, const vec2 &t2, const vec2 &t3 )
	{
		vertices[0] = v1;
		vertices[1] = v2;
		vertices[2] = v3;

		normals[0] = n1;
		normals[1] = n2;
		normals[2] = n3;

		textures[0] = t1;
		textures[1] = t2;
		textures[2] = t3;
	}
};