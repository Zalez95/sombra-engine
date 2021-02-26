#include "se/graphics/core/VertexBuffer.h"
#include "se/graphics/core/GLWrapper.h"

namespace se::graphics {

	VertexBuffer::VertexBuffer()
	{
		GL_WRAP( glGenBuffers(1, &mBufferId) );
		SOMBRA_TRACE_LOG << "Created VBO " << mBufferId;
	}


	VertexBuffer::VertexBuffer(VertexBuffer&& other) : mBufferId(other.mBufferId)
	{
		other.mBufferId = 0;
	}


	VertexBuffer::~VertexBuffer()
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteBuffers(1, &mBufferId) );
			SOMBRA_TRACE_LOG << "Deleted VBO " << mBufferId;
		}
	}


	VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other)
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteBuffers(1, &mBufferId) );
			SOMBRA_TRACE_LOG << "Deleted VBO " << mBufferId;
		}

		mBufferId = other.mBufferId;
		other.mBufferId = 0;

		return *this;
	}


	std::size_t VertexBuffer::size() const
	{
		int ret = 0;
		bind();
		GL_WRAP( glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &ret) );
		return ret;
	}


	void VertexBuffer::resizeAndCopy(const void* data, std::size_t size)
	{
		bind();
		GL_WRAP( glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW) );
	}


	void VertexBuffer::copy(const void* data, std::size_t size, std::size_t offset)
	{
		bind();
		GL_WRAP( glBufferSubData(GL_ARRAY_BUFFER, offset, size, data) );
	}


	void VertexBuffer::read(void** data, std::size_t size, std::size_t offset)
	{
		bind();
		GL_WRAP( glGetBufferSubData(GL_ARRAY_BUFFER, offset, static_cast<GLsizeiptr>(size), *data) );
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
