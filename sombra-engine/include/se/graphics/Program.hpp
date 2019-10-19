#ifndef PROGRAM_HPP
#define PROGRAM_HPP

namespace se::graphics {

	template <typename T>
	void Program::setUniform(const char* name, const T& value) const
	{
		setUniform(getUniformLocation(name), value);
	}


	template <typename T>
	void Program::setUniformV(const char* name, std::size_t count, const T* valuePtr) const
	{
		setUniformV(getUniformLocation(name), count, valuePtr);
	}

}

#endif		// PROGRAM_HPP
