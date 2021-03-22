#include <string>
#include <stdexcept>
#include "se/utils/FixedVector.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/Texture.h"
#include "GLWrapper.h"

namespace se::graphics {

	FrameBuffer::FrameBuffer(FrameBufferTarget target) : mTarget(target)
	{
		// Create the FBO
		GL_WRAP( glGenFramebuffers(1, &mBufferId) );
		SOMBRA_TRACE_LOG << "Created FBO " << mBufferId;
	}


	FrameBuffer::FrameBuffer(FrameBuffer&& other) :
		mBufferId(other.mBufferId), mTarget(other.mTarget)
	{
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
		mTarget = other.mTarget;
		other.mBufferId = 0;

		return *this;
	}


	FrameBuffer& FrameBuffer::getDefaultFrameBuffer()
	{
		static FrameBuffer instance(0, FrameBufferTarget::Both);
		return instance;
	}


	FrameBuffer& FrameBuffer::setTarget(FrameBufferTarget target)
	{
		mTarget = target;
		return *this;
	}


	FrameBuffer& FrameBuffer::setColorBuffer(bool active)
	{
		GLenum buf = active? GL_FRONT : GL_NONE;

		bind();
		GL_WRAP( glDrawBuffer(buf) );
		return *this;
	}


	FrameBuffer& FrameBuffer::attach(
		const Texture& texture, unsigned int attachment,
		int level, int layer, int orientation
	) {
		GLenum glTarget = toGLTextureTarget(texture.getTarget());
		GLenum glAttachment = toGLFrameBufferAttachment(attachment);

		bind();

		switch (texture.getTarget()) {
			case TextureTarget::Texture1D:
				GL_WRAP( glFramebufferTexture1D(GL_FRAMEBUFFER, glAttachment, glTarget, texture.getTextureId(), level) );
				break;
			case TextureTarget::Texture2D:
				GL_WRAP( glFramebufferTexture2D(GL_FRAMEBUFFER, glAttachment, glTarget, texture.getTextureId(), level) );
				break;
			case TextureTarget::Texture3D:
				GL_WRAP( glFramebufferTexture3D(GL_FRAMEBUFFER, glAttachment, glTarget, texture.getTextureId(), level, layer) );
				break;
			case TextureTarget::CubeMap:
				GL_WRAP( glFramebufferTexture2D(GL_FRAMEBUFFER, glAttachment, GL_TEXTURE_CUBE_MAP_POSITIVE_X + orientation, texture.getTextureId(), level) );
				break;
		}

		GL_WRAP( GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) );
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error("FrameBuffer error: status 0x" + std::to_string(status));
		}

		// Set the color attachments to render to
		if (attachment >= FrameBufferAttachment::kColor0) {
			unsigned int colorIndex = attachment - FrameBufferAttachment::kColor0;
			mColorAttachments[colorIndex] = true;

			utils::FixedVector<GLenum, FrameBufferAttachment::kMaxColorAttachments> glAttachments;
			for (unsigned int i = 0; i < FrameBufferAttachment::kMaxColorAttachments; ++i) {
				if (mColorAttachments[i]) {
					glAttachments.push_back( toGLFrameBufferAttachment(FrameBufferAttachment::kColor0 + i) );
				}
			}
			GL_WRAP( glDrawBuffers(static_cast<GLsizei>(glAttachments.size()), glAttachments.data()) );
		}

		unbind();

		return *this;
	}


	FrameBuffer& FrameBuffer::copy(
		FrameBuffer& other, const FrameBufferMask::Mask& mask,
		std::size_t x0, std::size_t y0, std::size_t w0, std::size_t h0,
		std::size_t x1, std::size_t y1, std::size_t w1, std::size_t h1,
		TextureFilter filter
	) {
		FrameBufferTarget oldTarget1 = mTarget;
		FrameBufferTarget oldTarget2 = other.mTarget;

		mTarget = FrameBufferTarget::Write;
		other.mTarget = FrameBufferTarget::Read;

		bind();
		other.bind();
		GL_WRAP( glBlitFramebuffer(
			static_cast<GLint>(x0), static_cast<GLint>(y0), static_cast<GLint>(x0 + w0), static_cast<GLint>(y0 + h0),
			static_cast<GLint>(x1), static_cast<GLint>(y1), static_cast<GLint>(x1 + w1), static_cast<GLint>(y1 + h1),
			toGLFrameBufferMask(mask),
			toGLFilter(filter)
		) );
		unbind();

		mTarget = oldTarget1;
		other.mTarget = oldTarget2;

		return *this;
	}


	void FrameBuffer::bind() const
	{
		GL_WRAP( glBindFramebuffer(toGLFrameBufferTarget(mTarget), mBufferId) );
	}


	void FrameBuffer::unbind() const
	{
		GL_WRAP( glBindFramebuffer(toGLFrameBufferTarget(mTarget), 0) );
	}

}
