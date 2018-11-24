#include "se/graphics/GLWrapper.h"
#include "se/graphics/buffers/VertexBuffer.h"

namespace se::graphics {

	VertexBuffer::VertexBuffer(
		const float* data,
		unsigned int count, unsigned int componentSize
	) : mComponentSize(componentSize)
	{
		GL_WRAP( glGenBuffers(1, &mBufferId) );

		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, mBufferId) );
		GL_WRAP( glBufferData(
			GL_ARRAY_BUFFER,
			count * sizeof(GLfloat),
			data,
			GL_STATIC_DRAW
		) );
		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	}


	VertexBuffer::VertexBuffer(
		const unsigned short* data,
		unsigned int count, unsigned int componentSize
	) : mComponentSize(componentSize)
	{
		GL_WRAP( glGenBuffers(1, &mBufferId) );

		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, mBufferId) );
		GL_WRAP( glBufferData(
			GL_ARRAY_BUFFER,
			count * sizeof(GLushort),
			data,
			GL_STATIC_DRAW
		) );
		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	}


	VertexBuffer::VertexBuffer(VertexBuffer&& other)
	{
		mBufferId = other.mBufferId;
		mComponentSize = other.mComponentSize;

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
		mComponentSize = other.mComponentSize;

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

}
