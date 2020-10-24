#include "se/graphics/core/VertexArray.h"
#include "GLWrapper.h"

namespace se::graphics {

	VertexArray::VertexArray()
	{
		GL_WRAP( glGenVertexArrays(1, &mArrayId) );
		SOMBRA_TRACE_LOG << "Created VAO " << mArrayId;
	}


	VertexArray::VertexArray(VertexArray&& other) : mArrayId(other.mArrayId)
	{
		other.mArrayId = 0;
	}


	VertexArray::~VertexArray()
	{
		if (mArrayId != 0) {
			GL_WRAP( glDeleteVertexArrays(1, &mArrayId) );
			SOMBRA_TRACE_LOG << "Deleted VAO " << mArrayId;
		}
	}


	VertexArray& VertexArray::operator=(VertexArray&& other)
	{
		if (mArrayId != 0) {
			GL_WRAP( glDeleteVertexArrays(1, &mArrayId) );
			SOMBRA_TRACE_LOG << "Deleted VAO " << mArrayId;
		}

		mArrayId = other.mArrayId;
		other.mArrayId = 0;

		return *this;
	}


	void VertexArray::setVertexAttribute(
		unsigned int index,
		TypeId type, bool normalized, int componentSize, std::size_t stride, std::size_t offset
	) const
	{
		GL_WRAP( glEnableVertexAttribArray(index) );

		if ((type == TypeId::Float) || (type == TypeId::HalfFloat)) {
			GL_WRAP( glVertexAttribPointer(
				index, componentSize, toGLType(type), normalized,
				static_cast<GLsizei>(stride), reinterpret_cast<const void*>(offset)
			) );
		}
		else if (type == TypeId::Double) {
			GL_WRAP( glVertexAttribLPointer(
				index, componentSize, toGLType(type),
				static_cast<GLsizei>(stride), reinterpret_cast<const void*>(offset)
			) );
		}
		else {
			GL_WRAP( glVertexAttribIPointer(
				index, componentSize, toGLType(type),
				static_cast<GLsizei>(stride), reinterpret_cast<const void*>(offset)
			) );
		}
	}


	void VertexArray::setAttributeDivisor(unsigned int index, unsigned int divisor) const
	{
		GL_WRAP( glVertexAttribDivisor(index, divisor) );
	}


	void VertexArray::bind() const
	{
		GL_WRAP( glBindVertexArray(mArrayId) );
	}


	void VertexArray::unbind() const
	{
		GL_WRAP( glBindVertexArray(0) );
	}

}
