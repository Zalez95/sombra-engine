#include "se/graphics/core/UniformBuffer.h"
#include "se/graphics/core/GLWrapper.h"

namespace se::graphics {

	UniformBuffer::UniformBuffer() : mSlot(0)
	{
		GL_WRAP( glGenBuffers(1, &mBufferId) );
		SOMBRA_TRACE_LOG << "Created UBO " << mBufferId;
	}


	UniformBuffer::UniformBuffer(UniformBuffer&& other) :
		mBufferId(other.mBufferId), mSlot(other.mSlot)
	{
		other.mBufferId = 0;
	}


	UniformBuffer::~UniformBuffer()
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteBuffers(1, &mBufferId) );
			SOMBRA_TRACE_LOG << "Deleted UBO " << mBufferId;
		}
	}


	UniformBuffer& UniformBuffer::operator=(UniformBuffer&& other)
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteBuffers(1, &mBufferId) );
			SOMBRA_TRACE_LOG << "Deleted UBO " << mBufferId;
		}

		mBufferId = other.mBufferId;
		mSlot = other.mSlot;
		other.mBufferId = 0;

		return *this;
	}


	void UniformBuffer::resizeAndCopy(const void* data, std::size_t size)
	{
		GL_WRAP( glBindBuffer(GL_UNIFORM_BUFFER, mBufferId) );
		GL_WRAP( glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW) );
	}


	void UniformBuffer::copy(const void* data, std::size_t size, std::size_t offset)
	{
		GL_WRAP( glBindBuffer(GL_UNIFORM_BUFFER, mBufferId) );
		GL_WRAP( glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data) );
	}


	void UniformBuffer::bind() const
	{
		GL_WRAP( glBindBufferBase(GL_UNIFORM_BUFFER, mSlot, mBufferId) );
	}


	void UniformBuffer::unbind() const
	{
		GL_WRAP( glBindBuffer(GL_UNIFORM_BUFFER, 0) );
	}

}