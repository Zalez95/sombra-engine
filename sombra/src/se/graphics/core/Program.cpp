#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Shader.h"
#include "GLWrapper.h"

namespace se::graphics {

	Program::Program()
	{
		GL_WRAP( mProgramId = glCreateProgram() );
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


	bool Program::load(const Shader* const* shaders, std::size_t shaderCount)
	{
		bool ret = true;

		// 1. Attach the shaders to the program
		for (std::size_t i = 0; i < shaderCount; ++i) {
			GL_WRAP( glAttachShader(mProgramId, shaders[i]->mShaderId) );
		}

		GL_WRAP( glLinkProgram(mProgramId) );

		// 2. Check program related errors
		GLint status;
		GL_WRAP( glGetProgramiv(mProgramId, GL_LINK_STATUS, &status) );
		if (status == GL_FALSE) {
			GLint infoLogLength;
			GL_WRAP( glGetProgramiv(mProgramId, GL_INFO_LOG_LENGTH, &infoLogLength) );

			char* infoLog = new char[infoLogLength + 1];
			GL_WRAP( glGetProgramInfoLog(mProgramId, infoLogLength, NULL, infoLog) );

			SOMBRA_ERROR_LOG << "Failed to link the program " << mProgramId << ":" << infoLog;

			delete[] infoLog;
			ret = false;
		}
		else {
			SOMBRA_TRACE_LOG << "Created Program " << mProgramId;
		}

		// 3. Remove the shaders from the program
		for (std::size_t i = 0; i < shaderCount; ++i) {
			GL_WRAP( glDetachShader(mProgramId, shaders[i]->mShaderId) );
		}

		return ret;
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
