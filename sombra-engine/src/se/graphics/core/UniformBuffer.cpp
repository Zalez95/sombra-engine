#include "se/graphics/core/UniformBuffer.h"
#include "se/graphics/core/GLWrapper.h"

namespace se::graphics {

	UniformBuffer::UniformBuffer()
	{
		GL_WRAP( glGenBuffers(1, &mBufferId) );
		SOMBRA_TRACE_LOG << "Created UBO " << mBufferId;
	}


	UniformBuffer::UniformBuffer(UniformBuffer&& other)
	{
		mBufferId = other.mBufferId;
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
		other.mBufferId = 0;

		return *this;
	}


	void UniformBuffer::setData(const void* data, std::size_t size)
	{
		GL_WRAP( glBindBuffer(GL_UNIFORM_BUFFER, mBufferId) );
		GL_WRAP( glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STATIC_DRAW) );
		GL_WRAP( glBindBuffer(GL_UNIFORM_BUFFER, 0) );
	}


	void UniformBuffer::bind(unsigned int slot) const
	{
		GL_WRAP( glBindBufferBase(GL_UNIFORM_BUFFER, slot, mBufferId) );
	}


	void UniformBuffer::unbind() const
	{
		GL_WRAP( glBindBuffer(GL_UNIFORM_BUFFER, 0) );
	}

}
