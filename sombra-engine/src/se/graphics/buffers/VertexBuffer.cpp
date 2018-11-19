#include "se/graphics/GLWrapper.h"
#include "se/graphics/buffers/VertexBuffer.h"

namespace se::graphics {

	VertexBuffer::VertexBuffer(
		const float* data,
		unsigned int count, unsigned int componentSize
	) : mComponentSize(componentSize)
	{
		GL_WRAP( glGenBuffers(1, &mBufferID) );

		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, mBufferID) );
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
		GL_WRAP( glGenBuffers(1, &mBufferID) );

		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, mBufferID) );
		GL_WRAP( glBufferData(
			GL_ARRAY_BUFFER,
			count * sizeof(GLushort),
			data,
			GL_STATIC_DRAW
		) );
		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	}


	VertexBuffer::~VertexBuffer()
	{
		GL_WRAP( glDeleteBuffers(1, &mBufferID) );
	}


	void VertexBuffer::bind() const
	{
		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, mBufferID) );
	}


	void VertexBuffer::unbind() const
	{
		GL_WRAP( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	}

}
