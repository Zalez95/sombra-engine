#include "se/graphics/GLWrapper.h"
#include "se/graphics/buffers/IndexBuffer.h"

namespace se::graphics {

	IndexBuffer::IndexBuffer(const void* data, std::size_t size, TypeId type, std::size_t count) :
		mIndexType(type), mIndexCount(count)
	{
		createBuffer(data, size);
	}


	IndexBuffer::IndexBuffer(IndexBuffer&& other)
	{
		mBufferId = other.mBufferId;
		mIndexType = other.mIndexType;
		mIndexCount = other.mIndexCount;

		other.mBufferId = 0;
	}


	IndexBuffer::~IndexBuffer()
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteBuffers(1, &mBufferId) );
			SOMBRA_TRACE_LOG << "Deleted IBO " << mBufferId;
		}
	}


	IndexBuffer& IndexBuffer::operator=(IndexBuffer&& other)
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteBuffers(1, &mBufferId) );
		}

		mBufferId = other.mBufferId;
		mIndexType = other.mIndexType;
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

// Private functions
	void IndexBuffer::createBuffer(const void* data, std::size_t size)
	{
		GL_WRAP( glGenBuffers(1, &mBufferId) );

		GL_WRAP( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBufferId) );
		GL_WRAP( glBufferData(
			GL_ELEMENT_ARRAY_BUFFER,
			size,
			data,
			GL_STATIC_DRAW
		) );
		GL_WRAP( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );

		SOMBRA_TRACE_LOG << "Created IBO " << mBufferId;
	}

}
