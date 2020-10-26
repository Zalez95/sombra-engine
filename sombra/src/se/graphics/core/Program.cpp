#include <stdexcept>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Shader.h"
#include "GLWrapper.h"

namespace se::graphics {

	Program::Program(const Shader* const* shaders, std::size_t shaderCount)
	{
		// 1. Create the program
		GL_WRAP( mProgramId = glCreateProgram() );

		// 2. Attach the shaders to the program
		for (std::size_t i = 0; i < shaderCount; ++i) {
			GL_WRAP( glAttachShader(mProgramId, shaders[i]->mShaderId) );
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
			GL_WRAP( glDetachShader(mProgramId, shaders[i]->mShaderId) );
		}

		SOMBRA_TRACE_LOG << "Created Program " << mProgramId;
	}


	Program::Program(Program&& other) : mProgramId(other.mProgramId)
	{
		other.mProgramId = 0;
	}


	Program::~Program()
	{
		unbind();

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


	unsigned int Program::getAttributeLocation(const char* name) const
	{
		GLint ret = -1;

		bind();
		GL_WRAP( ret = glGetAttribLocation(mProgramId, name) );

		return ret;
	}


	void Program::bind() const
	{
		GL_WRAP( glUseProgram(mProgramId) );
	}


	void Program::unbind() const
	{
		GL_WRAP( glUseProgram(0) );
	}

}
