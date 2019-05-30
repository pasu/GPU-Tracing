//
#include "pch.h"
#include "textrenderer/textrenderer.h"
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

#include "./Material/RTMaterialManager.h"
#include "./Material/RTTextureManager.h"

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

	mat4 modelViewMatrix = glm::mat4( 1.0 );
	mat4 lastMatrix = glm::mat4( 1.0 );
	////////////////////////////////////////////////////////////////////
	// create scene
	int idx_texture = gTexManager.CreateTexture( "./data/Wood_Tower_Col.jpg" );

	int idx_material = gMaterialManager.CreateMaterial( vec3( 1 ), vec3( 0 ), DIFFUSE,
														0.0f, 2.5f,
														idx_texture,
														2.0f, 0.0f );

	Scene scene;
	scene.addMesh( "./data/wooden.dae", idx_material, vec3( 0, 0.25, -5 ), vec3( 3.1415f / 2.0f, 0.0f, 0.0f ) );

	idx_texture = gTexManager.CreateTexture( "./data/BeachStones.jpg" );
	idx_material = gMaterialManager.CreateMaterial( vec3( 1 ), vec3( 0 ), DIFFUSE,
													0.0f, 2.5f,
													idx_texture,
													2.0f, 0.0f );
	float y = 0.5;
	float z = 15;
	float width = 30;
	scene.addTriangle( vec3( 50, y, z ), vec3( -50, y, z ), vec3( -50, y, z - width ), idx_material,
					   vec3( 0, -1, 0 ), vec3( 0, -1, 0 ), vec3( 0, -1, 0 ),
					   vec2( 0, 0 ), vec2( 0, 1 ), vec2( 1, 1 ) );
	scene.addTriangle( vec3( 50, y, z - width ), vec3( 50, y, z ), vec3( -50, y, z - width ), idx_material,
					   vec3( 0, -1, 0 ), vec3( 0, -1, 0 ), vec3( 0, -1, 0 ),
					   vec2( 0, 0 ), vec2( 1, 1 ), vec2( 1, 0 ) );

	idx_material = gMaterialManager.CreateMaterial( vec3( 1, 0, 0 ), vec3( 300 ), DIFFUSE,
													0.0f, 2.5f,
													-1,
													2.0f, 0.0f );

	float x = -10;
	y = -20;
	z = 5;
	float offset = 5.5;

	scene.addAreaLight( vec3( x, y, z ), vec3( x, y, z - offset ),
						vec3( x + offset, y, z - offset ), vec3( x + offset, y, z ), idx_material, vec3( 0, 1, 0 ) );

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
	// SSBO
	RenderParameters rp;
	uint task_num = rp.nTaskNum;

	GLuint screenBuffer_ID, rayBuffer_ID, triangleBuffer_ID, bvhBuffer_ID,
		materialsBuffer_ID, texturesBuffer_ID, textureInfosBuffer_ID,
		lightsBuffer_ID, lightsNumBuffer_ID,
		queueCounter_ID;

	scene.buffer2GPU( screenBuffer_ID, rayBuffer_ID, triangleBuffer_ID, bvhBuffer_ID,
					  materialsBuffer_ID, texturesBuffer_ID, textureInfosBuffer_ID,
					  lightsBuffer_ID, lightsNumBuffer_ID, queueCounter_ID );

	//scene.savebuffer( "D://code//GPU-Tracing//scene" );

    //wf_test
    /////////////////////////////////////////////////////
	//GLuint wf_genRay_SID = loadcomputeshader( "./shader/wf_extension.glsl" );
    ////////////////////////////////////////////////////////
	GLuint wf_reset_SID = loadcomputeshader( "./shader/wf_reset.glsl" );
	GLuint genRay_SID = loadcomputeshader( "./shader/wf_rayGen.glsl" );
	GLuint wf_extension_SID = loadcomputeshader( "./shader/wf_extension.glsl" );
	////////////////////////////////////////////////////////////////////
	// hit with Ray
	GLuint texHandle = genTexture();
	GLuint tracing_SID = loadcomputeshader( "./shader/wf_logic.glsl" );
	///////////////////////////////////////////////////////////////////////

	GLuint quad_ID = loadshaders( "./shader/quad.vertexshader", "./shader/quad.fragmentshader" );
	GLuint _qtextureID = glGetUniformLocation( quad_ID, "texture_dds" );
	// The full screen quad's FBO
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
	RTCamera *camera = new RTCamera;
	setCameraSpeed( 0.1 );
	/////////////////////////////////////////////////////////
	while ( !glfwWindowShouldClose( window ) )
	{
		auto start = clock();
		static unsigned int frame_id = 0;
		static uint total_num = 0;
		static wf_queue_counter qc;

		updateCamera( *camera );
		camera->copyMCamera( modelViewMatrix );
		//modelViewMatrix = glm::inverse( modelViewMatrix );

		if ( lastMatrix != modelViewMatrix )
		{
			lastMatrix = modelViewMatrix;
			frame_id = 0;
		}

		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

		if ( frame_id == 0)
        {
			total_num = 0;
			//1 reset
			glUseProgram( wf_reset_SID );
			glUniform1ui( glGetUniformLocation( wf_reset_SID, "frame_id" ), frame_id );
			glDispatchCompute( task_num / LocalSize_X, 1, 1 ); // 800*800 threads in blocks of 256*1

            
			scene.getQueueCount( queueCounter_ID, qc );
            //2 gen ray
			glUseProgram( genRay_SID );

			glUniform1ui( glGetUniformLocation( genRay_SID, "total_num" ), total_num );
			glUniform1ui( glGetUniformLocation( genRay_SID, "frame_id" ), frame_id++ );
			glUniformMatrix4fv( glGetUniformLocation( genRay_SID, "mvMatrix" ), 1, GL_FALSE, &modelViewMatrix[0][0] );

			glDispatchCompute( task_num / LocalSize_X, 1, 1 ); // 800*800 threads in blocks of 256*1

			scene.getQueueCount( queueCounter_ID, qc );
			total_num += qc.raygenQueue;
			total_num = total_num % ( SCRWIDTH * SCRHEIGHT );

            //3 extension intersection
			glUseProgram( wf_extension_SID );
			glDispatchCompute( task_num / LocalSize_X, 1, 1 ); // 800*800 threads in blocks of 256*1

            scene.resetQueue( queueCounter_ID );
			scene.getQueueCount( queueCounter_ID, qc );
        }
        else
        {
			glUseProgram( tracing_SID );

			glUniform1ui( glGetUniformLocation( tracing_SID, "frame_id" ), frame_id );
			glUniformMatrix4fv( glGetUniformLocation( tracing_SID, "mvMatrix" ), 1, GL_FALSE, &modelViewMatrix[0][0] );
			glDispatchCompute( task_num / LocalSize_X, 1, 1 ); // 800*800 threads in blocks of 256*1

            //2 gen ray
			glUseProgram( genRay_SID );
			glUniform1ui( glGetUniformLocation( genRay_SID, "total_num" ), total_num );
			glUniform1ui( glGetUniformLocation( genRay_SID, "frame_id" ), frame_id++ );
			glUniformMatrix4fv( glGetUniformLocation( genRay_SID, "mvMatrix" ), 1, GL_FALSE, &modelViewMatrix[0][0] );
			glDispatchCompute( task_num / LocalSize_X, 1, 1 ); // 800*800 threads in blocks of 256*1

			scene.getQueueCount( queueCounter_ID, qc );
			total_num += qc.raygenQueue;
			total_num = total_num % ( SCRWIDTH * SCRHEIGHT );

			//3 extension intersection
			glUseProgram( wf_extension_SID );
			glDispatchCompute( task_num / LocalSize_X, 1, 1 ); // 800*800 threads in blocks of 256*1

            scene.resetQueue( queueCounter_ID );
        }
		
		glUseProgram( quad_ID );

		glBindVertexArray( vao );
		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, texHandle );
		glUniform1i( _qtextureID, 0 );

		glEnableVertexAttribArray( 0 );
		glBindBuffer( GL_ARRAY_BUFFER, quad_vertexbuffer );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );

		glDrawArrays( GL_TRIANGLES, 0, 6 );

		glDisableVertexAttribArray( 0 );

		auto end = clock();
		double diff = end - start;
		//cout << 1000.0 / diff << endl;
		float fps = 1000.0 / diff;
		processInput( window );

		qp::renderText( std::to_string( fps ) + " fps", 5, 20, 20, {0.5,0.5,0.5} );
		glfwSwapBuffers( window );
		glfwPollEvents();
	}

	glDeleteVertexArrays( 1, &vao );

	glDeleteBuffers( 1, &quad_vertexbuffer );
	glDeleteProgram( quad_ID );

	glfwTerminate();

	return 0;
}