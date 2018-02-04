#include <string>
#include <stdexcept>
#include "fe/graphics/Shader.h"
#include "fe/graphics/GLWrapper.h"

namespace fe { namespace graphics {

	Shader::Shader(const char* source, unsigned int shaderType)
	{
		// 1. Create the Shader
		mShaderID = glCreateShader(shaderType);
		GL_WRAP( glShaderSource(mShaderID, 1, &source, NULL) );

		// 2. Compile the shader
		GL_WRAP( glCompileShader(mShaderID) );

		// 3. Check shader related errors
		int status;
		GL_WRAP( glGetShaderiv(mShaderID, GL_COMPILE_STATUS, &status) );
		if (status == GL_FALSE) {
			GLint infoLogLength;
			GL_WRAP( glGetShaderiv(mShaderID, GL_INFO_LOG_LENGTH, &infoLogLength) );

			char* infoLog = new char[infoLogLength + 1];
			GL_WRAP( glGetShaderInfoLog(mShaderID, infoLogLength, NULL, infoLog) );

			std::string strInfoLog = "Failed to create the shader \"" + std::to_string(shaderType) + "\"\n" + std::string(infoLog);
			delete[] infoLog;

			throw std::runtime_error(strInfoLog);
		}
	}


	Shader::~Shader()
	{
		GL_WRAP( glDeleteShader(mShaderID) );
	}

}}
