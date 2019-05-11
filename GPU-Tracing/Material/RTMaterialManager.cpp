#include "RTMaterialManager.h"

#include <algorithm>

RTMaterialManager::RTMaterialManager()
{
}

RTMaterialManager::~RTMaterialManager()
{
	ClearAll();
}

int RTMaterialManager::CreateMaterial( const vec3 &color,
													 const vec3 &emission,
													 const ShadingType shadingType,
													 const float &reflectionFactor,
													 const float &indexOfRefraction,
													 const float &_pow,
													 const float &_k )
{
	int nCode = RTMaterialManager::getHashCode( color, emission, shadingType, reflectionFactor, indexOfRefraction, _pow, _k );

    vector<int>::iterator it = find( mKeys.begin(), mKeys.end(), nCode );
	
    int idx = -1;

    if (it != mKeys.end())
    {
		idx = std::distance( mKeys.begin(), it );
		return idx;
    }

    mKeys.push_back( nCode );
	mMaterials.push_back( RTMaterial( color, emission, shadingType, reflectionFactor, indexOfRefraction, _pow, _k ) );

    return mKeys.size() - 1;
}

void RTMaterialManager::ClearAll()
{
	mMaterials.clear();
	mKeys.clear();
}

int RTMaterialManager::getHashCode( const vec3 &color,
									const vec3 &emission,
									const ShadingType shadingType,
									const float &reflectionFactor,
									const float &indexOfRefraction,
									const float &_pow,
									const float &_k )
{
	std::hash<std::string> h;

	char buffer[64];
	sprintf( buffer,
			 "%.2f,%.2f,%.2f;\
        %.2f,%.2f,%.2f;\
        %d;\
        %.2f;\
        %.2f;\
        %.2f;\
        %.2f",
			 color.x, color.y, color.z,
			 emission.x, emission.y, emission.z,
			 (int)shadingType,
			 reflectionFactor,
			 indexOfRefraction,
			 _pow,
			 _k );

	int code = h( buffer );
	return code;
}
