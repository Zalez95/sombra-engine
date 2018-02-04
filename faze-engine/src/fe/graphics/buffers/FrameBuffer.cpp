#include <string>
#include <stdexcept>
#include "fe/graphics/GLWrapper.h"
#include "fe/graphics/buffers/FrameBuffer.h"

namespace fe { namespace graphics {

	FrameBuffer::FrameBuffer()
	{
		// Create the FBO
		GL_WRAP( glGenFramebuffers(1, &mBufferID) );
		GL_WRAP( glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mBufferID) );

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error("FB error, status: 0x" + std::to_string(status));
		}

		// Restore the default FBO
		GL_WRAP( glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0) );
	}


	FrameBuffer::~FrameBuffer()
	{
		GL_WRAP( glDeleteFramebuffers(1, &mBufferID) );
	}


	void FrameBuffer::bindForReading() const
	{
		GL_WRAP( glBindFramebuffer(GL_READ_FRAMEBUFFER, mBufferID) );
	}


	void FrameBuffer::bindForWriting() const
	{
		GL_WRAP( glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mBufferID) );
	}

}}
