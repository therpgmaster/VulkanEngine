#pragma once
#include <cmath>

namespace Math 
{
	// returns multiple of x that is closest to v
	static float closestMultiple(float v, float x) 
	{
		if (x > v) { return x; }
		v = v + (x / 2);
		v = v - fmod(v, x);
		return v;
	}

	static float invSqrt(const float& v) { return 1.f / sqrt(v); }
	static double invSqrt(const double& v) { return 1.0 / sqrt(v); }

} // namespace Math


