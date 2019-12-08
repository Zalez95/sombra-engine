#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "Constants.h"

namespace se::graphics {

	class Texture;


	/**
	 * Class FrameBuffer, it's used for creating, binding and unbinding Frame
	 * Buffer Objects
	 */
	class FrameBuffer
	{
	private:	// Attributes
		/** The id of the Frame Buffer Object */
		unsigned int mBufferId;

	public:		// Functions
		/** Creates a new FrameBuffer */
		FrameBuffer();
		FrameBuffer(const FrameBuffer& other) = delete;
		FrameBuffer(FrameBuffer&& other);

		/** Class destructor */
		~FrameBuffer();

		/** Assignment operator */
		FrameBuffer& operator=(const FrameBuffer& other) = delete;
		FrameBuffer& operator=(FrameBuffer&& other);

		/** Attachs the given Texture to the current FrameBuffer so the result
		 * of the write operations will be stored into that Texture
		 *
		 * @param	texture the Texture to Attach to the FrameBuffer
		 * @param	attachment the attachment id of the texture (stencil,
		 *			depth, color...)
		 * @param	colorIndex when the attachment is of Color type, it
		 *			specifies the color attachment index where the texture will
		 *			be attached */
		void attach(
			const Texture& texture,
			FrameBufferAttachment attachment, unsigned int colorIndex = 0
		) const;

		/** Binds the Frame Buffer Object
		 *
		 * @param	target the operation that we want to bind the FrameBuffer
		 *			to */
		void bind(FrameBufferTarget target = FrameBufferTarget::Both) const;

		/** Unbinds the Frame Buffer Object */
		void unbind() const;
	};

}

#endif		// FRAME_BUFFER_H
