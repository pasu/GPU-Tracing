#pragma once

#include "../pch.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;

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

struct AABB
{
	vec3 aabb_minaabb_min;
	vec3 aabb_minaabb_max;

	AABB( const vec3 &min, const vec3 &max )
	{
		aabb_minaabb_min = min;
		aabb_minaabb_max = max;
	}

    vec3 getCentroid() const
	{
		vec3 v = ( aabb_minaabb_min + aabb_minaabb_max ) * 0.5f;
		return v;
	};

    void expandToInclude( const vec3 &p )
	{
		aabb_minaabb_min = ::min( aabb_minaabb_min, p );
		aabb_minaabb_max = ::max( aabb_minaabb_max, p );
	}

	void expandToInclude( const AABB &b )
	{
		for ( int i = 0; i < 3; ++i )
		{
			if ( b.aabb_minaabb_min[i] < aabb_minaabb_min[i] )
				aabb_minaabb_min[i] = b.aabb_minaabb_min[i];
			if ( b.aabb_minaabb_max[i] > aabb_minaabb_max[i] )
				aabb_minaabb_max[i] = b.aabb_minaabb_max[i];
		}
	}

    uint32_t maxDimension() const
	{
		vec3 extent = aabb_minaabb_max - aabb_minaabb_min;

		uint32_t result = 0;
		if ( extent.y > extent.x )
		{
			result = 1;
			if ( extent.z > extent.y ) result = 2;
		}
		else if ( extent.z > extent.x )
			result = 2;

		return result;
	}

    float surfaceArea() const
	{
		vec3 extent = aabb_minaabb_max - aabb_minaabb_min;
		return 2.f * ( extent.x * extent.z + extent.x * extent.y + extent.y * extent.z );
	}
};

struct RTTriangle
{
	vec3 vertices[3];
	vec3 normals[3];
	vec2 textures[3];
	int mIndex;

	RTTriangle( const vec3 &v1, const vec3 &v2, const vec3 &v3, const vec3 &n1, const vec3 &n2, const vec3 &n3, const vec2 &t1, const vec2 &t2, const vec2 &t3, const int& idx )
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

        mIndex = idx;
	}

    RTTriangle(const RTTriangle& other)
    {
		if ( this == &other )
			return;

        vertices[0] = other.vertices[0];
		vertices[1] = other.vertices[1];
		vertices[2] = other.vertices[2];

		normals[0] = other.normals[0];
		normals[1] = other.normals[1];
		normals[2] = other.normals[2] ;

		textures[0] = other.textures[0];
		textures[1] = other.textures[1];
		textures[2] = other.textures[2];

		mIndex = other.mIndex;
    }

    AABB getAABB()
    {
		vec3 min, max;
		min = max = vertices[0];

		min = ::min( min, vertices[1] );
		max = ::max( max, vertices[1] );

		min = ::min( min, vertices[2] );
		max = ::max( max, vertices[2] );

		return AABB( min, max );
    }
};