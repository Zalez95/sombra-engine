#ifndef PROGRAM_HPP
#define PROGRAM_HPP

#include <algorithm>

namespace se::graphics {

	template <typename T>
	void Program::setUniform(const char* name, const T& value) const
	{
		auto itUniform = std::lower_bound(mUniformLocations.begin(), mUniformLocations.end(), name, compare);
		if (itUniform != mUniformLocations.end()) {
			setUniform(itUniform->location, value);
		}
	}


	template <typename T>
	void Program::setUniformV(const char* name, std::size_t count, const T* valuePtr) const
	{
		auto itUniform = std::lower_bound(mUniformLocations.begin(), mUniformLocations.end(), name, compare);
		if (itUniform != mUniformLocations.end()) {
			setUniformV(itUniform->location, count, valuePtr);
		}
	}

}

#endif		// PROGRAM_HPP
