#include "Shader.h"
#include <string>
#include <stdexcept>

namespace graphics {

	Shader::Shader(const char* source, GLuint shaderType)
	{
		// 1. Create the Shader
		mShaderID = glCreateShader(shaderType);
		glShaderSource(mShaderID, 1, &source, NULL);

		// 2. Compile the shader
		glCompileShader(mShaderID);

		// 3. Check shader related errors
		int status;
		glGetShaderiv(mShaderID, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE) {
			GLint infoLogLength;
			glGetShaderiv(mShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);

			GLchar* infoLog = new GLchar[infoLogLength + 1];
			glGetShaderInfoLog(mShaderID, infoLogLength, NULL, infoLog);

			std::string strInfoLog = "Failed to create the shader \"" + std::to_string(shaderType) + "\"\n" + std::string(infoLog);
			delete[] infoLog;

			throw std::runtime_error(strInfoLog);
		}
	}


	Shader::~Shader()
	{
		glDeleteShader(mShaderID);
	}

}
