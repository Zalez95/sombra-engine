#include "fe/graphics/Program.h"
#include <string>
#include <stdexcept>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>
#include "fe/graphics/Shader.h"

namespace fe { namespace graphics {

	Program::Program(const std::vector<const Shader*>& shaders)
	{
		// 1. Create the program
		mProgramID = glCreateProgram();

		// 2. Attach the shaders to the program
		for (const Shader* shader : shaders) {
			glAttachShader(mProgramID, shader->getShaderID());
		}

		glLinkProgram(mProgramID);

		// 3. Check program related errors
		GLint status;
		glGetProgramiv(mProgramID, GL_LINK_STATUS, &status);
		if (status == GL_FALSE) {
			GLint infoLogLength;
			glGetProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);

			char* infoLog = new char[infoLogLength + 1];
			glGetProgramInfoLog(mProgramID, infoLogLength, NULL, infoLog);

			std::string strInfoLog = "Failed to compile the program\n" + std::string(infoLog);
			delete[] infoLog;

			throw std::runtime_error(strInfoLog);
		}

		// 4. Remove the shaders from the program
		for (const Shader* shader: shaders) {
			glDetachShader(mProgramID, shader->getShaderID());
		}
	}


	Program::~Program()
	{
		disable();
		glDeleteProgram(mProgramID);
	}


	unsigned int Program::getUniformLocation(const char* name) const
	{
		return glGetUniformLocation(mProgramID, name);
	}


	void Program::setUniform(const char* name, int value) const
	{
		glUniform1i(glGetUniformLocation(mProgramID, name), value);
	}


	void Program::setUniform(unsigned int location, int value) const
	{
		glUniform1i(location, value);
	}


	void Program::setUniform(const char* name, float value) const
	{
		glUniform1f(glGetUniformLocation(mProgramID, name), value);
	}


	void Program::setUniform(unsigned int location, float value) const
	{
		glUniform1f(location, value);
	}


	void Program::setUniform(const char* name, const RGBColor& color) const
	{
		glUniform3f(glGetUniformLocation(mProgramID, name),
					color.mR, color.mG, color.mB);
	}


	void Program::setUniform(unsigned int location, const RGBColor& color) const
	{
		glUniform3f(location, color.mR, color.mG, color.mB);
	}


	void Program::setUniform(const char* name, const RGBAColor& color) const
	{
		glUniform4f(glGetUniformLocation(mProgramID, name),
					color.mR, color.mG, color.mB, color.mA);
	}


	void Program::setUniform(unsigned int location, const RGBAColor& color) const
	{
		glUniform4f(location, color.mR, color.mG, color.mB, color.mA);
	}


	void Program::setUniform(const char* name, const glm::vec2& vector) const
	{
		glUniform2f(glGetUniformLocation(mProgramID, name),
					vector.x, vector.y);
	}


	void Program::setUniform(unsigned int location, const glm::vec2& vector) const
	{
		glUniform2f(location, vector.x, vector.y);
	}


	void Program::setUniform(const char* name, const glm::vec3& vector) const
	{
		glUniform3f(glGetUniformLocation(mProgramID, name),
					vector.x, vector.y, vector.z);
	}


	void Program::setUniform(unsigned int location, const glm::vec3& vector) const
	{
		glUniform3f(location, vector.x, vector.y, vector.z);
	}


	void Program::setUniform(const char* name, const glm::vec4& vector) const
	{
		glUniform4f(glGetUniformLocation(mProgramID, name),
					vector.x, vector.y, vector.z, vector.w);
	}


	void Program::setUniform(unsigned int location, const glm::vec4& vector) const
	{
		glUniform4f(location, vector.x, vector.y, vector.z, vector.w);
	}


	void Program::setUniform(const char* name, const glm::mat3& matrix) const
	{
		glUniformMatrix3fv( glGetUniformLocation(mProgramID, name),
							1, GL_FALSE, glm::value_ptr(matrix) );
	}


	void Program::setUniform(unsigned int location, const glm::mat3& matrix) const
	{
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}


	void Program::setUniform(const char* name, const glm::mat4& matrix) const
	{
		glUniformMatrix4fv( glGetUniformLocation(mProgramID, name),
							1, GL_FALSE, glm::value_ptr(matrix) );
	}


	void Program::setUniform(unsigned int location, const glm::mat4& matrix) const
	{
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}


	void Program::enable() const
	{
		glUseProgram(mProgramID);
	}


	void Program::disable()
	{
		glUseProgram(0);
	}

}}
