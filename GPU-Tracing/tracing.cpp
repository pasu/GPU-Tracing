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
	GLuint texHandle = genTexture();
	GLuint compute_ID = loadcomputeshader( "./shader/compute.glsl" );
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
	setCameraSpeed( 1 );
    /////////////////////////////////////////////////////////
	while ( !glfwWindowShouldClose( window ) )
	{
		static unsigned int frame_id = 0;
		frame_id++;

        updateCamera( *camera );
		camera->copyMCamera( modelViewMatrix );
		modelViewMatrix = glm::transpose( modelViewMatrix );

		auto start = std::chrono::system_clock::now();

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		glUseProgram( compute_ID );

		glUniform1ui( glGetUniformLocation( compute_ID, "frame_id" ), frame_id );
		glUniformMatrix4fv( glGetUniformLocation( compute_ID, "mvMatrix" ), 1, GL_FALSE, &modelViewMatrix[0][0] );

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