#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <array>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace se::utils {

	/** It combines multiple hash values
	 *
	 * @param	seed the previous hash value, it will be updated with the new
	 *			combined hash. Initially it should be 0
	 * @param	v the value to calculate its hash value
	 * @see		https://www.boost.org/doc/libs/1_35_0/doc/html/boost/
	 *			hash_combine_id241013.html */
	template <class T>
	inline void hash_combine(std::size_t& seed, const T& v)
	{ seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2); }


	/** Struct PairHash, used for calculating the hash value of pairs */
	struct PairHash
	{
		/** Calculates the hash value of the given pair
		 *
		 * @param	pair the pair to calculate its hash value
		 * @return	the hash value */
		template <typename T1, typename T2>
		std::size_t operator()(const std::pair<T1, T2>& pair) const
		{
			std::size_t ret = 0;
			hash_combine(ret, pair.first);
			hash_combine(ret, pair.second);
			return ret;
		}
	};


	/** Calculates the pow with the given numbers
	 *
	 * @param	base the base of the pow function
	 * @param	exponent the exponent of the pow function
	 * @return	the value of the base raised to the exponent power */
	template <typename T>
	constexpr T ipow(T base, unsigned int exponent)
	{ return (exponent == 0)? 1 : base * ipow(base, exponent - 1); }


	/** Linearly interpolates the given values
	 *
	 * @param	a the initial value to interpolate
	 * @param	b the final value to interpolate
	 * @param	f the value used for interpolating between a and b */
	template <typename T>
	T lerp(T a, T b, T f)
	{ return a + f * (b - a); }

}

#endif		// MATH_UTILS_H
