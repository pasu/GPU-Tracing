#pragma once

#include "../pch.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;

class RTTexture
{
  public:
	RTTexture();
	~RTTexture();

	void LoadTextureImage( const char *a_File );
	int getBufferLength()const ;
  public:
	vec3 *m_Buffer;
	int m_Width, m_Height;
};
