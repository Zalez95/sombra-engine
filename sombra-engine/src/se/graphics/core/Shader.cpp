#include <string>
#include <stdexcept>
#include "se/graphics/core/Shader.h"
#include "GLWrapper.h"

namespace se::graphics {

	Shader::Shader(const char* source, ShaderType shaderType)
	{
		// 1. Create the Shader
		GL_WRAP( mShaderId = glCreateShader( toGLShader(shaderType) ) );
		GL_WRAP( glShaderSource(mShaderId, 1, &source, NULL) );

		// 2. Compile the shader
		GL_WRAP( glCompileShader(mShaderId) );

		// 3. Check shader related errors
		int status;
		GL_WRAP( glGetShaderiv(mShaderId, GL_COMPILE_STATUS, &status) );
		if (status == GL_FALSE) {
			GLint infoLogLength;
			GL_WRAP( glGetShaderiv(mShaderId, GL_INFO_LOG_LENGTH, &infoLogLength) );

			char* infoLog = new char[infoLogLength + 1];
			GL_WRAP( glGetShaderInfoLog(mShaderId, infoLogLength, NULL, infoLog) );

			std::string strInfoLog = "Failed to create the shader \"" + std::to_string(toGLShader(shaderType)) + "\"\n" + std::string(infoLog);
			delete[] infoLog;

			throw std::runtime_error(strInfoLog);
		}

		SOMBRA_TRACE_LOG << "Created Shader " << mShaderId;
	}


	Shader::Shader(Shader&& other) : mShaderId(other.mShaderId)
	{
		other.mShaderId = 0;
	}


	Shader::~Shader()
	{
		if (mShaderId != 0) {
			GL_WRAP( glDeleteShader(mShaderId) );
			SOMBRA_TRACE_LOG << "Deleted Shader " << mShaderId;
		}
	}


	Shader& Shader::operator=(Shader&& other)
	{
		if (mShaderId != 0) {
			GL_WRAP( glDeleteShader(mShaderId) );
			SOMBRA_TRACE_LOG << "Deleted Shader " << mShaderId;
		}

		mShaderId = other.mShaderId;
		other.mShaderId = 0;

		return *this;
	}

}
