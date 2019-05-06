#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;

#include <algorithm>

using namespace std;

class Utils {
public:
	static const float MAX_FLOAT;
	static const float MIN_FLOAT;
	static const float RT_PI;
	static const float EPSILON_FLOAT;
	static const float INV_PI;

	static float fresnel( const vec3 &I, const vec3 &N, const float refractionIndex )
	{
		float cosi = Utils::clamp_rt( dot( I, N ), -1.0f, 1.0f );
		float etai = 1, etat = refractionIndex;
		if ( cosi > 0 )
			std::swap( etai, etat );

		//Snell's law
		float sint = etai / etat * sqrtf( std::max( 0.0f, 1.0f - cosi * cosi ) );

		// Total internal reflection
		if ( sint >= 1 )
		{
			return 1.0f;
		}
		else
		{
			float cost = sqrtf( std::max( 0.0f, 1.0f - sint * sint ) );
			cosi = fabsf( cosi );
			float Rs = ( ( etat * cosi ) - ( etai * cost ) ) / ( ( etat * cosi ) + ( etai * cost ) );
			float Rp = ( ( etai * cosi ) - ( etat * cost ) ) / ( ( etai * cosi ) + ( etat * cost ) );
			return ( Rs * Rs + Rp * Rp ) / 2.0f;
		}
	}

	static vec3 refract( const vec3 &I, const vec3 &N, const float refractionIndex, bool& bOut) 
	{
		float cosi = Utils::clamp_rt( dot( I, N ), -1.0f, 1.0f );
		float etai = 1, etat = refractionIndex;
		vec3 n = N;
		if ( cosi < 0.0f )
			cosi = -cosi;
		else
		{
			bOut = false;
			std::swap( etai, etat );
			n = -N;
		}
		float eta = etai / etat;
		float k = 1.0f - eta * eta * ( 1 - cosi * cosi );

		//k < 0 = total internal reflection
		return k < 0.0f ? vec3( 0.0f ) : ( eta * I ) + ( eta * cosi - sqrtf( k ) ) * n;
	}

	static bool solveQuadratic(const float a, const float b, const float c, float &r0, float &r1);

	inline static float clamp_rt(const float v, const float lo, const float hi) {
		return std::max(lo, std::min(hi, v));
	}

	inline static float degToRad(const float deg) {
		return ( RT_PI / 180.0f * deg );
	}

	inline static bool floatEquals(const float value, const float comp, const float epsilon = EPSILON_FLOAT) {
		return abs(value - comp) <= epsilon;
	}
};