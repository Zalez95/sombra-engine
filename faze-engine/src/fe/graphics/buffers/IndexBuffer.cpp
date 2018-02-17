#include "fe/graphics/GLWrapper.h"
#include "fe/graphics/buffers/IndexBuffer.h"

namespace fe { namespace graphics {

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