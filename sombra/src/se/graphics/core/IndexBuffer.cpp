#include "se/graphics/core/IndexBuffer.h"
#include "GLWrapper.h"

namespace se::graphics {

	IndexBuffer::IndexBuffer() :
		mIndexType(TypeId::Byte), mIndexCount(0)
	{
		GL_WRAP( glGenBuffers(1, &mBufferId) );
		SOMBRA_TRACE_LOG << "Created IBO " << mBufferId;
	}


	IndexBuffer::IndexBuffer(IndexBuffer&& other) :
		mBufferId(other.mBufferId), mIndexType(other.mIndexType), mIndexCount(other.mIndexCount)
	{
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
			SOMBRA_TRACE_LOG << "Deleted IBO " << mBufferId;
		}

		mBufferId = other.mBufferId;
		mIndexType = other.mIndexType;
		mIndexCount = other.mIndexCount;

		other.mBufferId = 0;

		return *this;
	}


	std::size_t IndexBuffer::size() const
	{
		int ret = 0;
		bind();
		GL_WRAP( glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &ret) );
		return ret;
	}


	void IndexBuffer::resizeAndCopy(const void* data, std::size_t size, TypeId type, std::size_t count)
	{
		mIndexType = type;
		mIndexCount = count;

		bind();
		GL_WRAP( glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW) );
	}


	void IndexBuffer::copy(const void* data, std::size_t size, std::size_t offset)
	{
		bind();
		GL_WRAP( glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data) );
	}


	void IndexBuffer::read(void** data, std::size_t size, std::size_t offset)
	{
		bind();
		GL_WRAP( glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, static_cast<GLsizeiptr>(size), *data) );
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
