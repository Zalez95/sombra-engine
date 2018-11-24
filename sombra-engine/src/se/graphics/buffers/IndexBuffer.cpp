#include "se/graphics/GLWrapper.h"
#include "se/graphics/buffers/IndexBuffer.h"

namespace se::graphics {

	IndexBuffer::IndexBuffer(const unsigned short* data, unsigned int count) :
		mIndexCount(count)
	{
		GL_WRAP( glGenBuffers(1, &mBufferId) );

		GL_WRAP( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId) );
		GL_WRAP( glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			mIndexCount * sizeof(unsigned short),
			data,
			GL_STATIC_DRAW
		) );
		GL_WRAP( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
	}


	IndexBuffer::IndexBuffer(IndexBuffer&& other)
	{
		mBufferId = other.mBufferId;
		mIndexCount = other.mIndexCount;

		other.mBufferId = 0;
	}


	IndexBuffer::~IndexBuffer()
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteBuffers(1, &mBufferId) );
		}
	}


	IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other)
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteBuffers(1, &mBufferId) );
		}

		mBufferId = other.mBufferId;
		mIndexCount = other.mIndexCount;

		other.mBufferId = 0;

		return *this;
	}


	void IndexBuffer::bind() const
	{
		GL_WRAP( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId) );
	}


	void IndexBuffer::unbind() const
	{
		GL_WRAP( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
	}

}
