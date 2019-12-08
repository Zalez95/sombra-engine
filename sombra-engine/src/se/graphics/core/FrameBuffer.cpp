#include <string>
#include <stdexcept>
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/Texture.h"
#include "GLWrapper.h"

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
			SOMBRA_TRACE_LOG << "Deleted FBO " << mBufferId;
		}

		mBufferId = other.mBufferId;
		other.mBufferId = 0;

		return *this;
	}


	void FrameBuffer::attach(
		const Texture& texture,
		FrameBufferAttachment attachment, unsigned int colorIndex
	) const
	{
		GLenum glAttachment = GL_COLOR_ATTACHMENT0;
		switch (attachment) {
			case FrameBufferAttachment::Stencil:	glAttachment = GL_STENCIL_ATTACHMENT;				break;
			case FrameBufferAttachment::Depth:		glAttachment = GL_DEPTH_ATTACHMENT;					break;
			case FrameBufferAttachment::Color:		glAttachment = GL_COLOR_ATTACHMENT0 + colorIndex;	break;
		}

		GL_WRAP( glFramebufferTexture2D(
			GL_FRAMEBUFFER, glAttachment,
			GL_TEXTURE_2D, texture.getTextureId(), 0
		) );
	}


	void FrameBuffer::bind(FrameBufferTarget target) const
	{
		GLenum glTarget = GL_FRAMEBUFFER;
		switch (target) {
			case FrameBufferTarget::Read:	glTarget = GL_READ_FRAMEBUFFER;	break;
			case FrameBufferTarget::Write:	glTarget = GL_DRAW_FRAMEBUFFER;	break;
			case FrameBufferTarget::Both:	glTarget = GL_FRAMEBUFFER;		break;
		}

		GL_WRAP( glBindFramebuffer(glTarget, mBufferId) );
	}


	void FrameBuffer::unbind() const
	{
		GL_WRAP( glBindFramebuffer(GL_FRAMEBUFFER, 0) );
	}

}
