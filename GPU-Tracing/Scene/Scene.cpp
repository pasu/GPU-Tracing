#include "Scene.h"
#include "../shared.h"

Scene::Scene()
{
	mBTree = NULL;
}

Scene::~Scene()
{
    if (mBTree)
    {
		delete mBTree;
		mBTree = NULL;
    }
}

void Scene::addMesh( const char *a_File, const vec3 &pos, const vec3 &rotation, const vec3 &scale, const int &materialIndex )
{
	RTMesh *pM = new RTMesh( a_File, materialIndex );

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

void Scene::addTriangle( const vec3 &v1, const vec3 &v2, const vec3 &v3, const vec3 &n1, const vec3 &n2, const vec3 &n3, const vec2 &t1, const vec2 &t2, const vec2 &t3, const int &materialIndex )
{
	arrTriangles.push_back( RTTriangle(v1,v2,v3,n1,n2,n3,t1,t2,t3, materialIndex) );
}

void Scene::buffer2GPU( GLuint &rayBuffer_ID, GLuint &triangleBuffer_ID, GLuint &bvhBuffer_ID )
{
	BuildBVHTree();

	glGenBuffers( 1, &rayBuffer_ID );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, rayBuffer_ID );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( RTRay ) * SCRWIDTH * SCRHEIGHT, NULL, GL_STATIC_READ );

    glGenBuffers( 1, &triangleBuffer_ID );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, triangleBuffer_ID );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( RTTriangle ) * arrTriangles.size(), &arrTriangles[0], GL_STATIC_DRAW );


    glGenBuffers( 1, &bvhBuffer_ID );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, bvhBuffer_ID );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( BVHNode_32 ) * mBTree->getNodesCount(), mBTree->bvhTree, GL_STATIC_DRAW );


    int count = (int)arrTriangles.size();
    GLuint tricount_ID;
	glGenBuffers( 1, &tricount_ID );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, tricount_ID );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( int ), &count, GL_STATIC_DRAW );


    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, rayBuffer_ID );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, triangleBuffer_ID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, bvhBuffer_ID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3, tricount_ID );
}

void Scene::BuildBVHTree()
{
	if ( mBTree )
	{
		delete mBTree;
	}

	mBTree = new BVH( arrTriangles );
}
