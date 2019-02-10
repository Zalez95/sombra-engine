#include "se/graphics/GLWrapper.h"
#include "se/graphics/buffers/VertexBuffer.h"

namespace se::graphics {

	VertexBuffer::VertexBuffer(const void* data, std::size_t size)
	{
		createBuffer(data, size);
	}


	VertexBuffer::VertexBuffer(VertexBuffer&& other)
	{
		mBufferId = other.mBufferId;
		other.mBufferId = 0;
	}


	VertexBuffer::~VertexBuffer()
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteBuffers(1, &mBufferId) );
		}
	}


	VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other)
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteBuffers(1, &mBufferId) );
		}

		mBufferId = other.mBufferId;
		other.mBufferId = 0;

		return *this;
	}


	void VertexBuffer::bind() const
	{
		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, mBufferId) );
	}


	void VertexBuffer::unbind() const
	{
		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	}

// Private functions
	void VertexBuffer::createBuffer(const void* data, std::size_t size)
	{
		GL_WRAP( glGenBuffers(1, &mBufferId) );

		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, mBufferId) );
		GL_WRAP( glBufferData(
			GL_ARRAY_BUFFER,
			size,
			data,
			GL_STATIC_DRAW
		) );
		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	}

}
