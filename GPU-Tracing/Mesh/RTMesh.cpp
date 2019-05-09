#include "./RTMesh.h"

#include <iostream>

RTMesh::RTMesh( const char *a_File )
{
	importer = new Assimp::Importer();

	importer->ReadFile( a_File, aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace | aiProcess_Triangulate |
									aiProcess_JoinIdenticalVertices );

	if ( !importer->GetScene() )
	{
		cout << "Error loading mesh: " << a_File << ". " << importer->GetErrorString();

        delete importer;
		importer = NULL;

		return;
	}

    modelMatrix = normalMatrix = glm::mat4( 1.0 );
}

RTMesh::~RTMesh()
{
	delete importer;
	importer = NULL;
}

void RTMesh::applyTransforms()
{
	mat4 rotateXYZ = glm::mat4( 1.0 );

	rotateXYZ = glm::rotate( rotateXYZ, rotation.x, glm::vec3( 1.0f, 0.0f, 0.0f ) );
	rotateXYZ = glm::rotate( rotateXYZ, rotation.y, glm::vec3( 0.0f, 1.0f, 0.0f ) );
	rotateXYZ = glm::rotate( rotateXYZ, rotation.z, glm::vec3( 0.0f, 0.0f, 1.0f ) );

	modelMatrix = modelMatrix * rotateXYZ;

    modelMatrix = glm::translate( modelMatrix, pos );

    modelMatrix = glm::scale( modelMatrix, scale );

    const aiScene *scene = importer->GetScene();
	for ( unsigned int meshI = 0; meshI < scene->mNumMeshes; ++meshI )
	{
		aiMesh *mesh = scene->mMeshes[meshI];
		aiVector3D *vertices = mesh->mVertices;
		aiVector3D *normals = mesh->mNormals;
		for ( unsigned int i = 0; i < mesh->mNumVertices; ++i )
		{
			//TODO: unify vector usage
			vec4 pos( vertices[i][0], vertices[i][1], vertices[i][2], 1 );
			pos = modelMatrix * pos;
			vertices[i][0] = pos.x;
			vertices[i][1] = pos.y;
			vertices[i][2] = pos.z;

			vec4 n( normals[i][0], normals[i][1], normals[i][2], 0 );
			n = normalMatrix * n;
			normals[i][0] = n.x;
			normals[i][1] = n.y;
			normals[i][2] = n.z;
		}
	}
}

void RTMesh::getTriangles( vector<RTTriangle *> &triangleList )
{
	const aiScene *scene = importer->GetScene();

	for ( unsigned int meshI = 0; meshI < scene->mNumMeshes; ++meshI )
	{
		aiMesh *mesh = scene->mMeshes[meshI];
		aiFace *faces = mesh->mFaces;

		for ( unsigned int faceI = 0; faceI < mesh->mNumFaces; ++faceI )
		{
			vec3 vertices[3];
			vec3 normals[3];
			vec2 texCoords[3];

			aiVector3D &a = mesh->mVertices[faces[faceI].mIndices[0]];
			aiVector3D &b = mesh->mVertices[faces[faceI].mIndices[1]];
			aiVector3D &c = mesh->mVertices[faces[faceI].mIndices[2]];
			vertices[0] = vec3( a.x, a.y, a.z );
			vertices[1] = vec3( b.x, b.y, b.z );
			vertices[2] = vec3( c.x, c.y, c.z );

			aiVector3D &na = mesh->mNormals[faces[faceI].mIndices[0]];
			aiVector3D &nb = mesh->mNormals[faces[faceI].mIndices[1]];
			aiVector3D &nc = mesh->mNormals[faces[faceI].mIndices[2]];
			normals[0] = vec3( na.x, na.y, na.z );
			normals[1] = vec3( nb.x, nb.y, nb.z );
			normals[2] = vec3( nc.x, nc.y, nc.z );

			if ( mesh->mTextureCoords[0] != NULL )
			{
				aiVector3D &ta = mesh->mTextureCoords[0][faces[faceI].mIndices[0]];
				aiVector3D &tb = mesh->mTextureCoords[0][faces[faceI].mIndices[1]];
				aiVector3D &tc = mesh->mTextureCoords[0][faces[faceI].mIndices[2]];

				texCoords[0] = vec2( ta.x, ta.y );
				texCoords[1] = vec2( tb.x, tb.y );
				texCoords[2] = vec2( tc.x, tc.y );
			}
			else
			{
				texCoords[0] = texCoords[1] = texCoords[2] = vec2( 0 );
			}

			triangleList.push_back( new RTTriangle( vertices[0], vertices[1], vertices[2],
													normals[0], normals[1], normals[2],
													texCoords[0], texCoords[1], texCoords[2]) );
		}
	}
}
