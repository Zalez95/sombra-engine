#include "GBuffer.h"
#include <string>

namespace graphics {

	GBuffer::GBuffer(GLuint width, GLuint height)
	{
		// Create the FBO
		glGenFramebuffers(1, &mFrameBufferID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFrameBufferID);

		// Create the GBuffer textures
		glGenTextures(GBUFFER_NUM_TEXTURES, mTextureIDs);
		for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; ++i) {
			glBindTexture(GL_TEXTURE_2D, mTextureIDs[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, mTextureIDs[i], 0);
		}

		glGenTextures(1, &mDepthTextureID);
		glBindTexture(GL_TEXTURE_2D, mDepthTextureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTextureID, 0);

		GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, drawBuffers);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (status != GL_FRAMEBUFFER_COMPLETE) {
			throw ("FB error, status: 0x" + std::to_string(status) + "\n");
		}

		// restore default FBO
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	}


	GBuffer::~GBuffer()
	{
		glDeleteTextures(1, &mDepthTextureID);
		glDeleteTextures(GBUFFER_NUM_TEXTURES, mTextureIDs);
		glDeleteFramebuffers(1, &mFrameBufferID);
	}


	void GBuffer::bindForReading() const
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mFrameBufferID);
	}


	void GBuffer::bindForWriting() const
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFrameBufferID);
	}


	void GBuffer::setReadBuffer(GBUFFER_TEXTURE_TYPE textureType)
	{
		glReadBuffer(GL_COLOR_ATTACHMENT0 + textureType);
	}

}
