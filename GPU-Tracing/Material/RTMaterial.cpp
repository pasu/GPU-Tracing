#include "RTMaterial.h"

 RTMaterial::RTMaterial( const vec3 &color, const vec3 &emission,
     const ShadingType shadingType,
     const float& reflectionFactor, const float& indexOfRefraction,
     const float& _pow,
     const float &_k)
	: color( color ), emission( emission ),
	  shadingType( shadingType ),
	  reflectionFactor( reflectionFactor ),
	  indexOfRefraction( indexOfRefraction ),
	  m_pow(_pow),
	  m_k(_k)
 {
 }

 RTMaterial::~RTMaterial()
{
}
