#include "Scene.h"
#include "../shared.h"

Scene::Scene()
{
}

Scene::~Scene()
{

}

void Scene::addMesh( const char *a_File, const vec3 &pos, const vec3 &rotation, const vec3 &scale )
{
	RTMesh *pM = new RTMesh( a_File );

	pM->setPosition( pos );
	pM->setRotation( rotation );
	pM->setScale( scale );
	pM->applyTransforms();

    vector<RTTriangle *> tri_array;
	pM->getTriangles( tri_array );

    for ( size_t i = 0; i < tri_array.size(); i++ )
	{
		arrTriangles.push_back( *tri_array[i] );
	}

    delete pM;
	pM = NULL;
}

void Scene::addTriangle( const vec3 &v1, const vec3 &v2, const vec3 &v3, const vec3 &n1, const vec3 &n2, const vec3 &n3, const vec2 &t1, const vec2 &t2, const vec2 &t3 )
{
	arrTriangles.push_back( RTTriangle(v1,v2,v3,n1,n2,n3,t1,t2,t3) );
}

void Scene::buffer2GPU( GLuint &rayBuffer_ID, GLuint &triangleBuffer_ID )
{
	glGenBuffers( 1, &rayBuffer_ID );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, rayBuffer_ID );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( RTRay ) * SCRWIDTH * SCRHEIGHT, NULL, GL_STATIC_READ );

    glGenBuffers( 1, &triangleBuffer_ID );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, triangleBuffer_ID );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( RTTriangle ) * arrTriangles.size(), &arrTriangles[0], GL_STATIC_DRAW );

    int offset = 0;
	//glBufferSubData( GL_SHADER_STORAGE_BUFFER, offset, sizeof( RTTriangle ), &arrTriangles[0] );
	offset = sizeof( RTTriangle );

    int count = arrTriangles.size();
    GLuint tricount_ID;
	glGenBuffers( 1, &tricount_ID );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, tricount_ID );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( int ), &count, GL_STATIC_DRAW );


    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, rayBuffer_ID );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, triangleBuffer_ID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, tricount_ID );
}
