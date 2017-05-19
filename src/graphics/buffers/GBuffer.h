#ifndef Gs_BUFFER_H
#define Gs_BUFFER_H

#include <GL/glew.h>

namespace graphics {

	enum GBUFFER_TEXTURE_TYPE {
		GBUFFER_TEXTURE_TYPE_POSITION,
		GBUFFER_TEXTURE_TYPE_DIFFUSE,
		GBUFFER_TEXTURE_TYPE_NORMAL,
		GBUFFER_TEXTURE_TYPE_TEXCOORD,
		GBUFFER_NUM_TEXTURES
	};


	/** Class GBuffer, it's used for creating, binding and unbinding Frame Buffer Objects */
	class GBuffer
	{
	private:	// Attributes
		/** The ID of the Frame Buffer Object */
		GLuint mFrameBufferID;
		
		/** The ID of the Depth Texture */
		GLuint mDepthTextureID;

		/** The ID of the depth	render buffer */
		GLuint mTextureIDs[GBUFFER_NUM_TEXTURES];

	public:		// Functions
		/** Creates a new GBuffer
		 * 
		 * @param	width the width of the texture of the GBuffer
		 * @param	height the height of the texture of the GBuffer */
		GBuffer(GLuint width, GLuint height);

		/** Class destructor */
		~GBuffer();

		/** Binds the Frame Buffer Object */
		void bindForReading() const;

		/** Binds the Frame Buffer Object */
		void bindForWriting() const;

		/** Sets the texture of the frame buffer from which we are going to
		 * read from
		 * 
		 * @param	textureType the type of the texture from wchich we want to
		 *			read from */
		void setReadBuffer(GBUFFER_TEXTURE_TYPE textureType);
	};

}

#endif		// G_BUFFER_H
