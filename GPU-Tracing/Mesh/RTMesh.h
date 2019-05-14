#pragma once

#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "../DS/datastructure.h"
#include <vector>

using namespace std;

class RTMesh
{
    public:
	    RTMesh( const char *a_File, const int& materialIndex );
	    ~RTMesh();

        void applyTransforms();
		void getTriangles( vector<RTTriangle *> &triangleList );

        inline void setPosition( const vec3& _pos )
		{
			pos = _pos;
		}
		inline void setRotation( const vec3 &_rotation )
		{
			rotation = _rotation;
		}
		inline void setScale( const vec3 &_scale)
		{
			scale = _scale;
		}

		inline void computeNormalMatrix()
		{
			normalMatrix = glm::inverse( modelMatrix );
			//normalMatrix = glm::transpose( normalMatrix );
		};

    private:
	    Assimp::Importer *importer;

        vec3 pos;
        vec3 scale;
		vec3 rotation;

		mat4 modelMatrix;  //Object to world Matrix
		mat4 normalMatrix; //Tranposed Inverse Model Matrix

        int mIndex;
};