#pragma once

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#ifndef GLM_GTC_matrix_transform
#define GLM_GTC_matrix_transform
#endif

#ifndef GLM_GTX_transform
#define GLM_GTX_transform
#endif

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif

#define GLM_GTC_quaternion
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/quaternion.hpp>
#include<glm/ext.hpp>
#include <glad/glad.h>
#include <string>
#include <vector>
namespace qp
{

GLuint createShader( const char *source, GLenum type, const char *errinfo = "" );
void linkProgram( GLuint program, const char *errinfo = "" );

// compute shader
GLuint createProgram_C( std::string source );

// vertex+fragment shader
GLuint createProgram_VF( std::string v_source, std::string f_source );

// vertex + tess_control + tess_eval + fragment shader
GLuint createProgram_TESS( std::string v_source, std::string tc_source, std::string te_source, std::string f_source );

// the basic default render pipeline
// don't use it in product
// vertex_data = {vec3 vVertex, vec3 vNormal, vec3 vColor}
void renderTriangles( const std::vector<glm::vec3> &vertex_data, const glm::mat4 &mViewProjection, const glm::vec3 &vLight={-1,-1,-1}, const glm::mat4 &mModel = glm::identity<glm::mat4>() );
} // namespace qp