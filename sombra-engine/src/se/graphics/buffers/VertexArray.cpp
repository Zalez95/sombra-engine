#include "se/graphics/GLWrapper.h"
#include "se/graphics/buffers/VertexArray.h"
#include "se/graphics/buffers/VertexBuffer.h"

namespace se::graphics {

	VertexArray::VertexArray()
	{
		GL_WRAP( glGenVertexArrays(1, &mArrayId) );
	}


	VertexArray::VertexArray(VertexArray&& other)
	{
		mArrayId = other.mArrayId;
		other.mArrayId = 0;
	}


	VertexArray::~VertexArray()
	{
		if (mArrayId != 0) {
			GL_WRAP( glDeleteVertexArrays(1, &mArrayId) );
		}
	}


	VertexArray& VertexArray::operator=(VertexArray&& other)
	{
		if (mArrayId != 0) {
			GL_WRAP( glDeleteVertexArrays(1, &mArrayId) );
		}

		mArrayId = other.mArrayId;
		other.mArrayId = 0;

		return *this;
	}


	void VertexArray::addBuffer(
		unsigned int index, const VertexBuffer& vertexBuffer,
		TypeId type, bool normalized, int componentSize, int stride
	) {
		bind();

		vertexBuffer.bind();
		GL_WRAP( glEnableVertexAttribArray(index) );
		GL_WRAP( glVertexAttribPointer(index, componentSize, toGLType(type), normalized, stride, 0) );

		unbind();
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
