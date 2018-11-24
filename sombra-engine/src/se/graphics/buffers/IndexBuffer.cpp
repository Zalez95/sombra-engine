#include "se/graphics/GLWrapper.h"
#include "se/graphics/buffers/IndexBuffer.h"

namespace se::graphics {

	IndexBuffer::IndexBuffer(const unsigned short* data, unsigned int count) :
		mIndexCount(count)
	{
		GL_WRAP( glGenBuffers(1, &mBufferID) );

		GL_WRAP( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferID) );
		GL_WRAP( glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			mIndexCount * sizeof(unsigned short),
			data,
			GL_STATIC_DRAW
		) );
		GL_WRAP( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
	}


	IndexBuffer::~IndexBuffer()
	{
		GL_WRAP( glDeleteBuffers(1, &mBufferID) );
	}


	void IndexBuffer::bind() const
	{
		GL_WRAP( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferID) );
	}


	void IndexBuffer::unbind() const
	{
		GL_WRAP( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
	}

}
