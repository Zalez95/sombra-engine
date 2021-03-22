#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/Program.h"
#include "GLWrapper.h"

namespace se::graphics {

	IUniformVariable::IUniformVariable(const char* name, const Program& program) : mName(name), mUniformLocation(-1)
	{
		GL_WRAP( mUniformLocation = glGetUniformLocation(program.mProgramId, name) );

		if (mUniformLocation == -1) {
			SOMBRA_WARN_LOG << "Uniform variable \"" << name << "\" wasn't found in Program " << program.mProgramId;
		}
	}


	const std::string& IUniformVariable::getName() const
	{
		return mName;
	}


	bool IUniformVariable::found() const
	{
		return mUniformLocation >= 0;
	}


// Private functions
	template <>
	void IUniformVariable::setUniform<int>(const int& value) const
	{
		GL_WRAP( glUniform1i(mUniformLocation, value) );
	}


	template <>
	void IUniformVariable::setUniformV<int>(const int* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform1iv(mUniformLocation, static_cast<GLsizei>(count), valuePtr) );
	}


	template <>
	void IUniformVariable::setUniform<unsigned int>(const unsigned int& value) const
	{
		GL_WRAP( glUniform1ui(mUniformLocation, value) );
	}


	template <>
	void IUniformVariable::setUniformV<unsigned int>(const unsigned int* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform1uiv(mUniformLocation, static_cast<GLsizei>(count), valuePtr) );
	}


	template <>
	void IUniformVariable::setUniform<float>(const float& value) const
	{
		GL_WRAP( glUniform1f(mUniformLocation, value) );
	}


	template <>
	void IUniformVariable::setUniformV<float>(const float* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform1fv(mUniformLocation, static_cast<GLsizei>(count), valuePtr) );
	}


	template <>
	void IUniformVariable::setUniform<bool>(const bool& value) const
	{
		GL_WRAP( glUniform1i(mUniformLocation, value) );
	}


	template <>
	void IUniformVariable::setUniformV<bool>(const bool* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform1iv(mUniformLocation, static_cast<GLsizei>(count), reinterpret_cast<const GLint*>(valuePtr)) );
	}


	template <>
	void IUniformVariable::setUniform<glm::vec2>(const glm::vec2& value) const
	{
		GL_WRAP( glUniform2f(mUniformLocation, value.x, value.y) );
	}


	template <>
	void IUniformVariable::setUniformV<glm::vec2>(const glm::vec2* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform2fv(mUniformLocation, static_cast<GLsizei>(count), glm::value_ptr(*valuePtr)) );
	}


	template <>
	void IUniformVariable::setUniform<glm::ivec2>(const glm::ivec2& value) const
	{
		GL_WRAP( glUniform2i(mUniformLocation, value.x, value.y) );
	}


	template <>
	void IUniformVariable::setUniformV<glm::ivec2>(const glm::ivec2* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform2iv(mUniformLocation, static_cast<GLsizei>(count), glm::value_ptr(*valuePtr)) );
	}


	template <>
	void IUniformVariable::setUniform<glm::vec3>(const glm::vec3& value) const
	{
		GL_WRAP( glUniform3f(mUniformLocation, value.x, value.y, value.z) );
	}


	template <>
	void IUniformVariable::setUniformV<glm::vec3>(const glm::vec3* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform3fv(mUniformLocation, static_cast<GLsizei>(count), glm::value_ptr(*valuePtr)) );
	}


	template <>
	void IUniformVariable::setUniform<glm::ivec3>(const glm::ivec3& value) const
	{
		GL_WRAP( glUniform3i(mUniformLocation, value.x, value.y, value.z) );
	}


	template <>
	void IUniformVariable::setUniformV<glm::ivec3>(const glm::ivec3* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform3iv(mUniformLocation, static_cast<GLsizei>(count), glm::value_ptr(*valuePtr)) );
	}


	template <>
	void IUniformVariable::setUniform<glm::vec4>(const glm::vec4& value) const
	{
		GL_WRAP( glUniform4f(mUniformLocation, value.x, value.y, value.z, value.w) );
	}


	template <>
	void IUniformVariable::setUniformV<glm::vec4>(const glm::vec4* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform4fv(mUniformLocation, static_cast<GLsizei>(count), glm::value_ptr(*valuePtr)) );
	}


	template <>
	void IUniformVariable::setUniform<glm::ivec4>(const glm::ivec4& value) const
	{
		GL_WRAP( glUniform4i(mUniformLocation, value.x, value.y, value.z, value.w) );
	}


	template <>
	void IUniformVariable::setUniformV<glm::ivec4>(const glm::ivec4* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniform4iv(mUniformLocation, static_cast<GLsizei>(count), glm::value_ptr(*valuePtr)) );
	}


	template <>
	void IUniformVariable::setUniformV<glm::mat3>(const glm::mat3* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniformMatrix3fv(mUniformLocation, static_cast<GLsizei>(count), GL_FALSE, glm::value_ptr(*valuePtr)) );
	}


	template <>
	void IUniformVariable::setUniform<glm::mat3>(const glm::mat3& value) const
	{
		setUniformV(&value, 1);
	}


	template <>
	void IUniformVariable::setUniformV<glm::mat4>(const glm::mat4* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniformMatrix4fv(mUniformLocation, static_cast<GLsizei>(count), GL_FALSE, glm::value_ptr(*valuePtr)) );
	}


	template <>
	void IUniformVariable::setUniform<glm::mat4>(const glm::mat4& value) const
	{
		setUniformV(&value, 1);
	}


	template <>
	void IUniformVariable::setUniformV<glm::mat3x4>(const glm::mat3x4* valuePtr, std::size_t count) const
	{
		GL_WRAP( glUniformMatrix3x4fv(mUniformLocation, static_cast<GLsizei>(count), GL_FALSE, glm::value_ptr(*valuePtr)) );
	}


	template <>
	void IUniformVariable::setUniform<glm::mat3x4>(const glm::mat3x4& value) const
	{
		setUniformV(&value, 1);
	}

}
