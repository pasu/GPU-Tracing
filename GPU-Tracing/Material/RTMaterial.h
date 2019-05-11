#pragma once

#include "../pch.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;

enum ShadingType
{
	DIFFUSE = 1,
	REFLECTIVE = 2,
	Phong = DIFFUSE | 4,
	Glossy = DIFFUSE | 8,
	TRANSMISSIVE_AND_REFLECTIVE = 16,
	DIFFUSE_AND_REFLECTIVE = DIFFUSE | 32,
	MICROFACET = DIFFUSE | 64,
	TRANSMISSIVE = 128
};

class RTMaterial
{
public:
	RTMaterial( const vec3 &color, const vec3 &emission, const ShadingType shadingType, 
        const float& reflectionFactor, const float& indexOfRefraction, 
        const float &_pow, const float &_k );
  ~RTMaterial();

private:
  vec3 color;
  float reflectionFactor;

  vec3 emission;
  float indexOfRefraction;

  int shadingType;
  // glossy, phong and microface
  float m_pow;
  float m_k;

};