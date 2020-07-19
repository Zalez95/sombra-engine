#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include <array>
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

		/** The color attachments added to the FrameBuffer */
		std::array<
			bool, FrameBufferAttachment::kMaxColorAttachments
		> mColorAttachments;

	public:		// Functions
		/** Creates a new FrameBuffer
		 *
		 * @param	target the target that we want to bind the FrameBuffer to */
		FrameBuffer(FrameBufferTarget target = FrameBufferTarget::Both);
		FrameBuffer(const FrameBuffer& other) = delete;
		FrameBuffer(FrameBuffer&& other);

		/** Class destructor */
		~FrameBuffer();

		/** Assignment operator */
		FrameBuffer& operator=(const FrameBuffer& other) = delete;
		FrameBuffer& operator=(FrameBuffer&& other);

		/** @return	the FrameBuffer where graphics API draws to by default */
		static FrameBuffer& getDefaultFrameBuffer();

		/** Sets the target of the FrameBuffer
		 *
		 * @param	target the target that we want to bind the FrameBuffer to */
		FrameBuffer& setTarget(FrameBufferTarget target);

		/** Attachs the given Texture to the current FrameBuffer so the result
		 * of the write operations will be stored into that Texture
		 *
		 * @param	texture the Texture to Attach to the FrameBuffer
		 * @param	attachment the @see FrameBufferAttachment of the texture
		 * @param	colorIndex when the attachment is of Color type, it
		 *			specifies the color attachment index where the texture will
		 *			be attached
		 * @param	level specifies the mip map level of the texture to attach
		 *			to the framebuffer
		 * @param	layer specifies the layer of a 2-dimensional image within a
		 *			Texture3D
		 * @param	orientation which face of the CubeMap is going to be set
		 *			(0 = positive X, 1 = negative X, 2 = positive Y,
		 *			3 = negative Y, 4 = positive Z, 5 = negative Z) */
		FrameBuffer& attach(
			const Texture& texture, unsigned int attachment,
			int level = 0, int layer = 0, int orientation = 0
		);

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
