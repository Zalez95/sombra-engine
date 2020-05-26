#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "Bindable.h"
#include "Constants.h"

namespace se::graphics {

	class Texture;


	/**
	 * Class FrameBuffer, it's used for creating, binding and unbinding Frame
	 * Buffer Objects
	 */
	class FrameBuffer : public Bindable
	{
	private:	// Attributes
		/** The id of the Frame Buffer Object */
		unsigned int mBufferId;

		/** The operation that we want to bind the FrameBuffer to */
		FrameBufferTarget mTarget;

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

		/** @return	the FrameBuffer where graphics API draws to by default */
		static FrameBuffer& getDefaultFrameBuffer();

		/** Set the target operation where the FrameBuffer will be bound
		 *
		 * @param	target the operation that we want to bind the FrameBuffer
		 *			to */
		void setTarget(FrameBufferTarget target) { mTarget = target; };

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

		/** Binds the Frame Buffer Object */
		void bind() const override;

		/** Unbinds the Frame Buffer Object */
		void unbind() const override;
	private:
		/** Creates a new FrameBuffer
		 *
		 * @param	bufferId the id of the Frame Buffer Object
		 * @param	target the operation that we want to bind the FrameBuffer
		 *			to */
		FrameBuffer(unsigned int bufferId, FrameBufferTarget target) :
			mBufferId(bufferId), mTarget(target) {};
	};

}

#endif		// FRAME_BUFFER_H
