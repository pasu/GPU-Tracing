#pragma once

#include "RTMaterial.h"

#include <string>
#include <vector>

using namespace std;

class RTMaterialManager
{
  public:
	RTMaterialManager();
	~RTMaterialManager();

	int CreateMaterial( const vec3 &color, 
        const vec3 &emission, 
        const ShadingType shadingType,
        const float &reflectionFactor, 
        const float &indexOfRefraction,
        const int &_texID,
		const float &_pow, 
        const float &_k );

    int getMaterialList( RTMaterial *& pMaterial);

	void ClearAll();
  public:
	static int getHashCode( const vec3 &color,
					 const vec3 &emission,
					 const ShadingType shadingType,
					 const float &reflectionFactor,
					 const float &indexOfRefraction,
                     const int &_texID,
					 const float &_pow,
					 const float &_k );
	private:
	    vector<RTMaterial> mMaterials;
	    vector<int> mKeys;
};
