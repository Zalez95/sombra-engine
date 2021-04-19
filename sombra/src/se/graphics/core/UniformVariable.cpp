#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/Program.h"
#include "GLWrapper.h"

namespace se::graphics {

	IUniformVariable::IUniformVariable(const char* name, std::shared_ptr<Program> program) :
		mName(name), mProgram(program), mUniformLocation(-1)
	{
		GL_WRAP( mUniformLocation = glGetUniformLocation(mProgram->mProgramId, mName.c_str()) );

		if (mUniformLocation == -1) {
			SOMBRA_WARN_LOG << "Uniform variable \"" << mName << "\" wasn't found in Program " << program->mProgramId;
		}
	}


	bool IUniformVariable::found() const
	{
		return mUniformLocation >= 0;
	}


// Private functions
	template <>
	void UniformVariable<int>::setUniform(const int& value) const
	{
		GL_WRAP( glUniform1i(mUniformLocation, value) );
	}


	template <>
	void UniformVariable<int>::setUniformV(const int* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform1iv(mUniformLocation, static_cast<GLsizei>(count), valuePtr) );
	}


	template <>
	void UniformVariable<unsigned int>::setUniform(const unsigned int& value) const
	{
		GL_WRAP( glUniform1ui(mUniformLocation, value) );
	}


	template <>
	void UniformVariable<unsigned int>::setUniformV(const unsigned int* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform1uiv(mUniformLocation, static_cast<GLsizei>(count), valuePtr) );
	}


	template <>
	void UniformVariable<float>::setUniform(const float& value) const
	{
		GL_WRAP( glUniform1f(mUniformLocation, value) );
	}


	template <>
	void UniformVariable<float>::setUniformV(const float* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform1fv(mUniformLocation, static_cast<GLsizei>(count), valuePtr) );
	}


	template <>
	void UniformVariable<glm::vec2>::setUniform(const glm::vec2& value) const
	{
		GL_WRAP( glUniform2f(mUniformLocation, value.x, value.y) );
	}


	template <>
	void UniformVariable<glm::vec2>::setUniformV(const glm::vec2* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform2fv(mUniformLocation, static_cast<GLsizei>(count), glm::value_ptr(*valuePtr)) );
	}


	template <>
	void UniformVariable<glm::ivec2>::setUniform(const glm::ivec2& value) const
	{
		GL_WRAP( glUniform2i(mUniformLocation, value.x, value.y) );
	}


	template <>
	void UniformVariable<glm::ivec2>::setUniformV(const glm::ivec2* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform2iv(mUniformLocation, static_cast<GLsizei>(count), glm::value_ptr(*valuePtr)) );
	}


	template <>
	void UniformVariable<glm::vec3>::setUniform(const glm::vec3& value) const
	{
		GL_WRAP( glUniform3f(mUniformLocation, value.x, value.y, value.z) );
	}


	template <>
	void UniformVariable<glm::vec3>::setUniformV(const glm::vec3* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform3fv(mUniformLocation, static_cast<GLsizei>(count), glm::value_ptr(*valuePtr)) );
	}


	template <>
	void UniformVariable<glm::ivec3>::setUniform(const glm::ivec3& value) const
	{
		GL_WRAP( glUniform3i(mUniformLocation, value.x, value.y, value.z) );
	}


	template <>
	void UniformVariable<glm::ivec3>::setUniformV(const glm::ivec3* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform3iv(mUniformLocation, static_cast<GLsizei>(count), glm::value_ptr(*valuePtr)) );
	}


	template <>
	void UniformVariable<glm::vec4>::setUniform(const glm::vec4& value) const
	{
		GL_WRAP( glUniform4f(mUniformLocation, value.x, value.y, value.z, value.w) );
	}


	template <>
	void UniformVariable<glm::vec4>::setUniformV(const glm::vec4* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform4fv(mUniformLocation, static_cast<GLsizei>(count), glm::value_ptr(*valuePtr)) );
	}


	template <>
	void UniformVariable<glm::ivec4>::setUniform(const glm::ivec4& value) const
	{
		GL_WRAP( glUniform4i(mUniformLocation, value.x, value.y, value.z, value.w) );
	}


	template <>
	void UniformVariable<glm::ivec4>::setUniformV(const glm::ivec4* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform4iv(mUniformLocation, static_cast<GLsizei>(count), glm::value_ptr(*valuePtr)) );
	}


	template <>
	void UniformVariable<glm::mat3>::setUniformV(const glm::mat3* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniformMatrix3fv(mUniformLocation, static_cast<GLsizei>(count), GL_FALSE, glm::value_ptr(*valuePtr)) );
	}


	template <>
	void UniformVariable<glm::mat3>::setUniform(const glm::mat3& value) const
	{
		setUniformV(&value, 1);
	}


	template <>
	void UniformVariable<glm::mat4>::setUniformV(const glm::mat4* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniformMatrix4fv(mUniformLocation, static_cast<GLsizei>(count), GL_FALSE, glm::value_ptr(*valuePtr)) );
	}


	template <>
	void UniformVariable<glm::mat4>::setUniform(const glm::mat4& value) const
	{
		setUniformV(&value, 1);
	}


	template <>
	void UniformVariable<glm::mat3x4>::setUniformV(const glm::mat3x4* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniformMatrix3x4fv(mUniformLocation, static_cast<GLsizei>(count), GL_FALSE, glm::value_ptr(*valuePtr)) );
	}


	template <>
	void UniformVariable<glm::mat3x4>::setUniform(const glm::mat3x4& value) const
	{
		setUniformV(&value, 1);
	}

}
