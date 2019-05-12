#include "Scene.h"
#include "../shared.h"

#include "../Material/RTMaterialManager.h"
#include "../Material/RTTextureManager.h"

extern RTTextureManager gTexManager;
extern RTMaterialManager gMaterialManager;

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

void Scene::addMesh( const char *mesh_File, const int &materialIndex, const vec3 & pos, const vec3 &rotation, const vec3 &scale)
{
	RTMesh *pM = new RTMesh( mesh_File, materialIndex );

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

void Scene::buffer2GPU( GLuint &rayBuffer_ID, GLuint &triangleBuffer_ID, GLuint &bvhBuffer_ID,
						GLuint &materialsBuffer_ID,
						GLuint &texturesBuffer_ID,
						GLuint &textureInfosBuffer_ID)
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

    RTMaterial *pMaterial = NULL;
	int nSize = gMaterialManager.getMaterialList( pMaterial );
    glGenBuffers( 1, &materialsBuffer_ID );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, materialsBuffer_ID );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( RTMaterial ) * nSize, pMaterial, GL_STATIC_DRAW );

    vector<RTTexInfo> texInfos;
	unsigned char *fBuffer = NULL;
	nSize = gTexManager.generateTexInfoWithBuffer( texInfos, fBuffer );

    glGenBuffers( 1, &texturesBuffer_ID );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, texturesBuffer_ID );
	glBufferData( GL_SHADER_STORAGE_BUFFER, nSize, fBuffer, GL_STATIC_DRAW );

    delete fBuffer;
	fBuffer = NULL;

    glGenBuffers( 1, &textureInfosBuffer_ID );
	glBindBuffer( GL_SHADER_STORAGE_BUFFER, textureInfosBuffer_ID );
	glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof( RTTexInfo ) * texInfos.size(), &texInfos[0], GL_STATIC_DRAW );

    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 0, rayBuffer_ID );
    glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, triangleBuffer_ID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, bvhBuffer_ID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 3, materialsBuffer_ID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 4, texturesBuffer_ID );
	glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 5, textureInfosBuffer_ID );
}

void Scene::BuildBVHTree()
{
	if ( mBTree )
	{
		delete mBTree;
	}

	mBTree = new BVH( arrTriangles );
}
