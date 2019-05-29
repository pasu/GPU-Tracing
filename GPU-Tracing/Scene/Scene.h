#include "../pch.h"
#include "../Mesh/RTMesh.h"
#include "../BVH/BVH.h"

#include "../WFManager.h"


class Scene
{
  public:
	Scene();
	~Scene();

    void addMesh( const char *mesh_File, const int &materialIndex = 0, const vec3 &pos = vec3( 0.0, 0.0, 0.0 ), const vec3 &rotation = vec3( 0.0, 0.0, 0.0 ), const vec3 &scale = vec3( 1, 1, 1 ) );
	void addTriangle( const vec3 &v1, const vec3 &v2, const vec3 &v3,
					  const int &materialIndex = 0, 
        const vec3 &n1 = vec3( 0.0, 0.0, 0.0 ), 
        const vec3 &n2 = vec3( 0.0, 0.0, 0.0 ), 
        const vec3 &n3 = vec3( 0.0, 0.0, 0.0 ), 
        const vec2 &t1 = vec2( 0.0, 0.0 ), 
        const vec2 &t2 = vec2( 0.0, 0.0 ), 
        const vec2 &t3 = vec2( 0.0, 0.0 ));

    void addAreaLight( const vec3 &v1, const vec3 &v2, const vec3 &v3, const vec3 &v4, const int &materialIndex, const vec3 &normal );
	void buffer2GPU( GLuint &screenBuffer_ID, 
                     GLuint &rayBuffer_ID, GLuint &triangleBuffer_ID, GLuint &bvhBuffer_ID,
					 GLuint &materialsBuffer_ID,
					 GLuint &texturesBuffer_ID,
					 GLuint &textureInfosBuffer_ID,
					 GLuint &lightsBuffer_ID,
					 GLuint &lightsNumBuffer_ID );

    void savebuffer(const char* path);

    private:
	    void BuildBVHTree();
  private:
	vector<RTTriangle> arrTriangles;

    vector<vec4> mLightBoundary;
    
    BVH *mBTree;
};