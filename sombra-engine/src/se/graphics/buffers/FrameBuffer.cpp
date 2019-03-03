#include <string>
#include <stdexcept>
#include "se/graphics/GLWrapper.h"
#include "se/graphics/buffers/FrameBuffer.h"

namespace se::graphics {

	FrameBuffer::FrameBuffer()
	{
		// Create the FBO
		GL_WRAP( glGenFramebuffers(1, &mBufferId) );
		GL_WRAP( glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mBufferId) );

		GL_WRAP( GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) );
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error("FB error, status: 0x" + std::to_string(status));
		}

		// Restore the default FBO
		GL_WRAP( glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0) );

		SOMBRA_TRACE_LOG << "Created FBO " << mBufferId;
	}


	FrameBuffer::FrameBuffer(FrameBuffer&& other)
	{
		mBufferId = other.mBufferId;
		other.mBufferId = 0;
	}


	FrameBuffer::~FrameBuffer()
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteFramebuffers(1, &mBufferId) );
			SOMBRA_TRACE_LOG << "Deleted FBO " << mBufferId;
		}
	}


	FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other)
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteFramebuffers(1, &mBufferId) );
		}

		mBufferId = other.mBufferId;
		other.mBufferId = 0;

		return *this;
	}


	void FrameBuffer::bindForReading() const
	{
		GL_WRAP( glBindFramebuffer(GL_READ_FRAMEBUFFER, mBufferId) );
	}


	void FrameBuffer::bindForWriting() const
	{
		GL_WRAP( glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mBufferId) );
	}

}
