#include <string>
#include <stdexcept>
#include <glm/gtc/type_ptr.hpp>
#include "se/graphics/Program.h"
#include "se/graphics/Shader.h"
#include "se/graphics/GLWrapper.h"

namespace se::graphics {

	Program::Program(const std::vector<const Shader*>& shaders)
	{
		// 1. Create the program
		GL_WRAP( mProgramId = glCreateProgram() );

		// 2. Attach the shaders to the program
		for (const Shader* shader : shaders) {
			GL_WRAP( glAttachShader(mProgramId, shader->getShaderId()) );
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

			std::string strInfoLog = "Failed to compile the program\n" + std::string(infoLog);
			delete[] infoLog;

			throw std::runtime_error(strInfoLog);
		}

		// 4. Remove the shaders from the program
		for (const Shader* shader: shaders) {
			GL_WRAP( glDetachShader(mProgramId, shader->getShaderId()) );
		}
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
		}
	}


	Program& Program::operator=(Program&& other)
	{
		if (mProgramId != 0) {
			GL_WRAP( glDeleteProgram(mProgramId) );
		}

		mProgramId = other.mProgramId;
		other.mProgramId = 0;

		return *this;
	}


	unsigned int Program::getUniformLocation(const char* name) const
	{
		GL_WRAP( unsigned int uniformLocation = glGetUniformLocation(mProgramId, name) );
		return uniformLocation;
	}


	void Program::setUniform(const char* name, int value) const
	{
		GL_WRAP( glUniform1i(glGetUniformLocation(mProgramId, name), value) );
	}


	void Program::setUniform(unsigned int location, int value) const
	{
		GL_WRAP( glUniform1i(location, value) );
	}


	void Program::setUniform(const char* name, unsigned int value) const
	{
		GL_WRAP( glUniform1ui(glGetUniformLocation(mProgramId, name), value) );
	}


	void Program::setUniform(unsigned int location, unsigned int value) const
	{
		GL_WRAP( glUniform1ui(location, value) );
	}


	void Program::setUniform(const char* name, float value) const
	{
		GL_WRAP( glUniform1f(glGetUniformLocation(mProgramId, name), value) );
	}


	void Program::setUniform(unsigned int location, float value) const
	{
		GL_WRAP( glUniform1f(location, value) );
	}


	void Program::setUniform(const char* name, const glm::vec2& vector) const
	{
		GL_WRAP( glUniform2f(glGetUniformLocation(mProgramId, name), vector.x, vector.y) );
	}


	void Program::setUniform(unsigned int location, const glm::vec2& vector) const
	{
		GL_WRAP( glUniform2f(location, vector.x, vector.y) );
	}


	void Program::setUniform(const char* name, const glm::vec3& vector) const
	{
		GL_WRAP( glUniform3f(glGetUniformLocation(mProgramId, name), vector.x, vector.y, vector.z) );
	}


	void Program::setUniform(unsigned int location, const glm::vec3& vector) const
	{
		GL_WRAP( glUniform3f(location, vector.x, vector.y, vector.z) );
	}


	void Program::setUniform(const char* name, const glm::vec4& vector) const
	{
		GL_WRAP( glUniform4f(glGetUniformLocation(mProgramId, name), vector.x, vector.y, vector.z, vector.w) );
	}


	void Program::setUniform(unsigned int location, const glm::vec4& vector) const
	{
		GL_WRAP( glUniform4f(location, vector.x, vector.y, vector.z, vector.w) );
	}


	void Program::setUniform(const char* name, const glm::mat3& matrix) const
	{
		GL_WRAP( glUniformMatrix3fv(glGetUniformLocation(mProgramId, name), 1, GL_FALSE, glm::value_ptr(matrix)) );
	}


	void Program::setUniform(unsigned int location, const glm::mat3& matrix) const
	{
		GL_WRAP( glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix)) );
	}


	void Program::setUniform(const char* name, const glm::mat4& matrix) const
	{
		GL_WRAP( glUniformMatrix4fv(glGetUniformLocation(mProgramId, name), 1, GL_FALSE, glm::value_ptr(matrix)) );
	}


	void Program::setUniform(unsigned int location, const glm::mat4& matrix) const
	{
		GL_WRAP( glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix)) );
	}


	void Program::enable() const
	{
		GL_WRAP( glUseProgram(mProgramId) );
	}


	void Program::disable()
	{
		GL_WRAP( glUseProgram(0) );
	}

}
