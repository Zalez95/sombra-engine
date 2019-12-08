#ifndef PROGRAM_HPP
#define PROGRAM_HPP

namespace se::graphics {

	template <typename T>
	void Program::setUniform(const char* name, const T& value) const
	{
		auto itUniform = mUniformLocations.find(name);
		if (itUniform != mUniformLocations.end()) {
			setUniform(itUniform->second, value);
		}
	}


	template <typename T>
	void Program::setUniformV(const char* name, std::size_t count, const T* valuePtr) const
	{
		auto itUniform = mUniformLocations.find(name);
		if (itUniform != mUniformLocations.end()) {
			setUniformV(itUniform->second, count, valuePtr);
		}
	}

}

#endif		// PROGRAM_HPP
