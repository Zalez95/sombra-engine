#include "VertexBuffer.h"

namespace graphics {

	VertexBuffer::VertexBuffer(const GLfloat* data, GLuint count, GLuint componentSize) :
		mComponentSize(componentSize)
	{
		glGenBuffers(1, &mBufferID);

		glBindBuffer(GL_ARRAY_BUFFER, mBufferID);
		glBufferData(
			GL_ARRAY_BUFFER,
			count * sizeof(GLfloat),
			data,
			GL_STATIC_DRAW
		);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}


	VertexBuffer::VertexBuffer(const GLushort* data, GLuint count, GLuint componentSize) :
		mComponentSize(componentSize)
	{
		glGenBuffers(1, &mBufferID);

		glBindBuffer(GL_ARRAY_BUFFER, mBufferID);
		glBufferData(
			GL_ARRAY_BUFFER,
			count * sizeof(GLushort),
			data,
			GL_STATIC_DRAW
		);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}


	VertexBuffer::~VertexBuffer()
	{
		glDeleteBuffers(1, &mBufferID);
	}


	void VertexBuffer::bind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, mBufferID);
	}


	void VertexBuffer::unbind() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

}
