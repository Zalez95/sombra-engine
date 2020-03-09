#include <stdexcept>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Shader.h"
#include "GLWrapper.h"

namespace se::graphics {

	Program::Program(const Shader* shaders[], std::size_t shaderCount)
	{
		// 1. Create the program
		GL_WRAP( mProgramId = glCreateProgram() );

		// 2. Attach the shaders to the program
		for (std::size_t i = 0; i < shaderCount; ++i) {
			GL_WRAP( glAttachShader(mProgramId, shaders[i]->getShaderId()) );
		}

		GL_WRAP( glLinkProgram(mProgramId) );

		// 3. Check program related errors
		GLint status;
		GL_WRAP( glGetProgramiv(mProgramId, GL_LINK_STATUS, &status) );
		if (status == GL_FALSE) {
			GLint infoLogLength;
			GL_WRAP( glGetProgramiv(mProgramId, GL_INFO_LOG_LENGTH, &infoLogLength) );

			char* infoLog = new char[infoLogLength + 1];
			GL_WRAP( glGetProgramInfoLog(mProgramId, infoLogLength, NULL, infoLog) );

			std::string strInfoLog = "Failed to link the program\n" + std::string(infoLog);
			delete[] infoLog;

			throw std::runtime_error(strInfoLog);
		}

		// 4. Remove the shaders from the program
		for (std::size_t i = 0; i < shaderCount; ++i) {
			GL_WRAP( glDetachShader(mProgramId, shaders[i]->getShaderId()) );
		}

		SOMBRA_TRACE_LOG << "Created Program " << mProgramId;
	}


	Program::Program(Program&& other)
	{
		mProgramId = other.mProgramId;
		other.mProgramId = 0;
	}


	Program::~Program()
	{
		disable();

		if (mProgramId != 0) {
			GL_WRAP( glDeleteProgram(mProgramId) );
			SOMBRA_TRACE_LOG << "Deleted Program " << mProgramId;
		}
	}


	Program& Program::operator=(Program&& other)
	{
		if (mProgramId != 0) {
			GL_WRAP( glDeleteProgram(mProgramId) );
			SOMBRA_TRACE_LOG << "Deleted Program " << mProgramId;
		}

		mProgramId = other.mProgramId;
		other.mProgramId = 0;

		return *this;
	}


	bool Program::addUniform(const char* name)
	{
		GL_WRAP( int uniformLocation = glGetUniformLocation(mProgramId, name) );

		if (uniformLocation == -1) {
			SOMBRA_WARN_LOG << "Uniform variable \"" << name << "\" wasn't found";
			return false;
		}

		mUniformLocations.insert(
			std::lower_bound(mUniformLocations.begin(), mUniformLocations.end(), name, compare),
			NameLocation{ name, uniformLocation }
		);

		return true;
	}


	bool Program::addUniformBlock(const char* name)
	{
		GL_WRAP( int uniformLocation = glGetUniformBlockIndex(mProgramId, name) );

		if (uniformLocation == -1) {
			SOMBRA_WARN_LOG << "Uniform block \"" << name << "\" wasn't found";
			return false;
		}

		mUniformBlocks.insert(
			std::lower_bound(mUniformBlocks.begin(), mUniformBlocks.end(), name, compare),
			NameLocation{ name, uniformLocation }
		);

		return true;
	}


	void Program::setUniformBlock(const char* name, unsigned int blockIndex) const
	{
		auto itUniformBlock = std::lower_bound(mUniformBlocks.begin(), mUniformBlocks.end(), name, compare);
		if (itUniformBlock != mUniformBlocks.end()) {
			GL_WRAP( glUniformBlockBinding(mProgramId, blockIndex, itUniformBlock->location) );
		}
	}


	void Program::enable() const
	{
		GL_WRAP( glUseProgram(mProgramId) );
	}


	void Program::disable() const
	{
		GL_WRAP( glUseProgram(0) );
	}

// Private functions
	bool Program::compare(const NameLocation& nameLocation, const char* name)
	{
		return nameLocation.name < name;
	}


	template <>
	void Program::setUniform<int>(int location, const int& value) const
	{
		GL_WRAP( glUniform1i(location, value) );
	}


	template <>
	void Program::setUniformV<int>(int location, std::size_t count, const int* valuePtr) const
	{
		GL_WRAP( glUniform1iv(location, count, valuePtr) );
	}


	template <>
	void Program::setUniform<unsigned int>(int location, const unsigned int& value) const
	{
		GL_WRAP( glUniform1ui(location, value) );
	}


	template <>
	void Program::setUniformV<unsigned int>(int location, std::size_t count, const unsigned int* valuePtr) const
	{
		GL_WRAP( glUniform1uiv(location, count, valuePtr) );
	}


	template <>
	void Program::setUniform<float>(int location, const float& value) const
	{
		GL_WRAP( glUniform1f(location, value) );
	}


	template <>
	void Program::setUniformV<float>(int location, std::size_t count, const float* valuePtr) const
	{
		GL_WRAP( glUniform1fv(location, count, valuePtr) );
	}


	template <>
	void Program::setUniform<glm::vec2>(int location, const glm::vec2& value) const
	{
		GL_WRAP( glUniform2f(location, value.x, value.y) );
	}


	template <>
	void Program::setUniformV<glm::vec2>(int location, std::size_t count, const glm::vec2* valuePtr) const
	{
		GL_WRAP( glUniform2fv(location, count, glm::value_ptr(*valuePtr)) );
	}


	template <>
	void Program::setUniform<glm::vec3>(int location, const glm::vec3& value) const
	{
		GL_WRAP( glUniform3f(location, value.x, value.y, value.z) );
	}


	template <>
	void Program::setUniformV<glm::vec3>(int location, std::size_t count, const glm::vec3* valuePtr) const
	{
		GL_WRAP( glUniform3fv(location, count, glm::value_ptr(*valuePtr)) );
	}


	template <>
	void Program::setUniform<glm::vec4>(int location, const glm::vec4& value) const
	{
		GL_WRAP( glUniform4f(location, value.x, value.y, value.z, value.w) );
	}


	template <>
	void Program::setUniformV<glm::vec4>(int location, std::size_t count, const glm::vec4* valuePtr) const
	{
		GL_WRAP( glUniform4fv(location, count, glm::value_ptr(*valuePtr)) );
	}


	template <>
	void Program::setUniformV<glm::mat3>(int location, std::size_t count, const glm::mat3* valuePtr) const
	{
		GL_WRAP( glUniformMatrix3fv(location, count, GL_FALSE, glm::value_ptr(*valuePtr)) );
	}


	template <>
	void Program::setUniform<glm::mat3>(int location, const glm::mat3& value) const
	{
		setUniformV(location, 1, &value);
	}


	template <>
	void Program::setUniformV<glm::mat4>(int location, std::size_t count, const glm::mat4* valuePtr) const
	{
		GL_WRAP( glUniformMatrix4fv(location, count, GL_FALSE, glm::value_ptr(*valuePtr)) );
	}


	template <>
	void Program::setUniform<glm::mat4>(int location, const glm::mat4& value) const
	{
		setUniformV(location, 1, &value);
	}

}
