#include "fe/graphics/buffers/IndexBuffer.h"
#include <GL/glew.h>

namespace fe { namespace graphics {

	IndexBuffer::IndexBuffer(const unsigned short* data, unsigned int count) :
		mIndexCount(count)
	{
		glGenBuffers(1, &mBufferID);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferID);
		glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			mIndexCount * sizeof(unsigned short),
			data,
			GL_STATIC_DRAW
		);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}


	IndexBuffer::~IndexBuffer()
	{
		glDeleteBuffers(1, &mBufferID);
	}


	void IndexBuffer::bind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferID);
	}


	void IndexBuffer::unbind() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

}}
