#pragma once
#include <cmath>
#include <numeric>

namespace Math
{
	#define EPSILON_F std::numeric_limits<float>::epsilon()

	// returns multiple of x that is closest to v
	static float closestMultiple(float v, float x) 
	{
		if (x > v) { return x; }
		v = v + (x / 2);
		v = v - fmod(v, x);
		return v;
	}

	// returns multiple of m that is closest to but >= v
	static uint32_t roundUpToClosestMultiple(uint32_t v, uint32_t m) 
	{
		
		if (m == 0) { return v; }
		uint32_t remainder = v % m;
		if (remainder == 0) { return v; }
		return v + m - remainder;
	}

	template<typename T = float>
	static T invSqrt(const T& v) { return 1.0 / sqrt(v); }

} // namespace Math


