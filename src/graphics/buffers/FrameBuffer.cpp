#include "FrameBuffer.h"
#include <string>
#include <stdexcept>

namespace graphics {

	FrameBuffer::FrameBuffer()
	{
		// Create the FBO
		glGenFramebuffers(1, &mBufferID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mBufferID);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error("FB error, status: 0x" + std::to_string(status));
		}

		// Restore the default FBO
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}


	FrameBuffer::~FrameBuffer()
	{
		glDeleteFramebuffers(1, &mBufferID);
	}


	void FrameBuffer::bindForReading() const
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mBufferID);
	}


	void FrameBuffer::bindForWriting() const
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mBufferID);
	}

}

