//
#include "pch.h"
#include <iostream>

GLFWwindow *window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;

#include "shader.h"
#include "texture.h"

#include "shared.h"

#include <chrono>

#include "RTCamera.h"
#include "RTCameraController.h"

#include "./DS/datastructure.h"
#include "./Scene/Scene.h"

#include "./Material/RTTextureManager.h"
#include "./Material/RTMaterialManager.h"


RTTextureManager gTexManager;
RTMaterialManager gMaterialManager;

void processInput( GLFWwindow *window )
{
	if ( glfwGetKey( window, GLFW_KEY_ESCAPE ) == GLFW_PRESS )
	{
		glfwSetWindowShouldClose( window, true );
	}
}

void framebuffer_size_callback( GLFWwindow *window, int width, int height )
{
	glViewport( 0, 0, width, height );
}

int main()
{
	glfwInit();
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	window = glfwCreateWindow( SCRWIDTH, SCRHEIGHT, "GPU Path Tracing", NULL, NULL );

	if ( window == NULL )
	{
		std::cout << "Create Window Fail";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent( window );

	glfwSetFramebufferSizeCallback( window, framebuffer_size_callback );

	if ( !gladLoadGLLoader( (GLADloadproc)glfwGetProcAddress ) )
	{
		std::cout << "Load glad Fail";
		return -1;
	}

	double lastTime = glfwGetTime();

    glm:mat4 modelViewMatrix = glm::mat4( 1.0 );
    ////////////////////////////////////////////////////////////////////
    // create scene
	int idx_texture = gTexManager.CreateTexture( "./data/Wood_Tower_Col.jpg" );
	int idx_material = gMaterialManager.CreateMaterial( vec3( 1 ), vec3( 0 ), DIFFUSE_AND_REFLECTIVE,
        0.0f, 2.5f,
        idx_texture,
        2.0f,0.0f);

    Scene scene;
	scene.addMesh( "./data/wooden.dae", idx_material, vec3( 0, 0, -10 ) );
	/*
	float z = -800;
	float w = 100;
	vec3 v1 = vec3( -w, w, z );
	vec3 v2 = vec3( -w, -w, z );
	vec3 v3 = vec3( w, -w, z );
	vec3 v4 = vec3( w, w, z );
    scene.addTriangle( v1, v2, v3 );
	scene.addTriangle( v1, v3, v4 );
    */
	// SSAO
	GLuint rayBuffer_ID, triangleBuffer_ID, bvhBuffer_ID, 
        materialsBuffer_ID, texturesBuffer_ID,textureInfosBuffer_ID;

	scene.buffer2GPU( rayBuffer_ID, triangleBuffer_ID, bvhBuffer_ID, 
        materialsBuffer_ID, texturesBuffer_ID, textureInfosBuffer_ID );
	
	GLuint genRay_SID = loadcomputeshader( "./shader/genRay.glsl" );
	////////////////////////////////////////////////////////////////////
    // hit with Ray
	GLuint texHandle = genTexture();
	GLuint tracing_SID = loadcomputeshader( "./shader/tracingRay.glsl" );
	///////////////////////////////////////////////////////////////////////

	GLuint quad_ID = loadshaders( "./shader/quad.vertexshader", "./shader/quad.fragmentshader" );
	GLuint _qtextureID = glGetUniformLocation( quad_ID, "texture_dds" );
	// The fullscreen quad's FBO
	static const GLfloat g_quad_vertex_buffer_data[] = {
		-1.0f,
		-1.0f,
		0.0f,
		1.0f,
		-1.0f,
		0.0f,
		-1.0f,
		1.0f,
		0.0f,
		-1.0f,
		1.0f,
		0.0f,
		1.0f,
		-1.0f,
		0.0f,
		1.0f,
		1.0f,
		0.0f,
	};

	GLuint vao;
	glGenVertexArrays( 1, &vao );
	glBindVertexArray( vao );

	GLuint quad_vertexbuffer;
	glGenBuffers( 1, &quad_vertexbuffer );
	glBindBuffer( GL_ARRAY_BUFFER, quad_vertexbuffer );
	glBufferData( GL_ARRAY_BUFFER, sizeof( g_quad_vertex_buffer_data ), g_quad_vertex_buffer_data, GL_STATIC_DRAW );

    ////////////////////////////////////////////////////////
	RTCamera* camera = new RTCamera;
	setCameraSpeed( 0.1 );
    /////////////////////////////////////////////////////////
	while ( !glfwWindowShouldClose( window ) )
	{
		static unsigned int frame_id = 0;

        updateCamera( *camera );
		camera->copyMCamera( modelViewMatrix );
		modelViewMatrix = glm::transpose( modelViewMatrix );

		auto start = std::chrono::system_clock::now();

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glUseProgram( genRay_SID );

		glUniform1ui( glGetUniformLocation( genRay_SID, "frame_id" ), frame_id );
		glUniformMatrix4fv( glGetUniformLocation( genRay_SID, "mvMatrix" ), 1, GL_FALSE, &modelViewMatrix[0][0] );

		glDispatchCompute( SCRWIDTH / LocalSize_X, SCRHEIGHT / LocalSize_Y, 1 ); // 512^2 threads in blocks of 16^2

		glUseProgram( tracing_SID );

		glUniform1ui( glGetUniformLocation( tracing_SID, "frame_id" ), frame_id++ );
		glUniformMatrix4fv( glGetUniformLocation( tracing_SID, "mvMatrix" ), 1, GL_FALSE, &modelViewMatrix[0][0] );

		glDispatchCompute( SCRWIDTH / LocalSize_X, SCRHEIGHT / LocalSize_Y, 1 ); // 512^2 threads in blocks of 16^2

		glUseProgram( quad_ID );

		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, texHandle );
		glUniform1i( _qtextureID, 0 );

		glEnableVertexAttribArray( 0 );
		glBindBuffer( GL_ARRAY_BUFFER, quad_vertexbuffer );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

		glDrawArrays( GL_TRIANGLES, 0, 6 );

		glDisableVertexAttribArray( 0 );

		auto end = std::chrono::system_clock::now();
		std::chrono::duration<double> diff = end - start;
		//cout << diff.count() << endl;

		processInput( window );
		glfwSwapBuffers( window );
		glfwPollEvents();
	}

	glDeleteVertexArrays( 1, &vao );

	glDeleteBuffers( 1, &quad_vertexbuffer );
	glDeleteProgram( quad_ID );

	glfwTerminate();

	return 0;
}