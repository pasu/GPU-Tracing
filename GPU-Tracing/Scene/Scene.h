#include "../pch.h"
#include "../Mesh/RTMesh.h"
#include "../BVH/BVH.h"


class Scene
{
  public:
	Scene();
	~Scene();

    void addMesh( const char *a_File, const vec3 &pos = vec3( 0.0, 0.0, 0.0 ), const vec3 &rotation = vec3( 0.0, 0.0, 0.0 ), const vec3 &scale = vec3( 1, 1, 1 ), const int &materialIndex = 0 );
	void addTriangle( const vec3 &v1, const vec3 &v2, const vec3 &v3, const vec3 &n1 = vec3( 0.0, 0.0, 0.0 ), const vec3 &n2 = vec3( 0.0, 0.0, 0.0 ), const vec3 &n3 = vec3( 0.0, 0.0, 0.0 ), const vec2 &t1 = vec2( 0.0, 0.0 ), const vec2 &t2 = vec2( 0.0, 0.0 ), const vec2 &t3 = vec2( 0.0, 0.0 ), const int &materialIndex = 0 );
	void buffer2GPU( GLuint &rayBuffer_ID, GLuint &triangleBuffer_ID, GLuint &bvhBuffer_ID );

    private:
	    void BuildBVHTree();
  private:
	vector<RTTriangle> arrTriangles;
    
    BVH *mBTree;
};