#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include <array>
#include <bitset>
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

		/** @return	the current Target of the FrameBuffer */
		FrameBufferTarget getTarget() const { return mTarget; };

		/** Sets the target of the FrameBuffer
		 *
		 * @param	target the target that we want to bind the FrameBuffer to
		 * @return	a reference to the current FrameBuffer */
		FrameBuffer& setTarget(FrameBufferTarget target);

		/** Enables or disables writting to the color buffer
		 * (Enabled by default)
		 *
		 * @param	active if the color buffer should be enabled or not */
		FrameBuffer& setColorBuffer(bool active);

		/** @return	true if the color buffer is enabled, false otherwise */
		bool getColorBuffer() const;

		/** Attachs the given Texture to the current FrameBuffer so the result
		 * of the write operations will be stored into that Texture
		 *
		 * @param	texture the Texture to Attach to the FrameBuffer. If it's
		 *			nullptr the texture attachment will be removed
		 * @param	attachment the @see FrameBufferAttachment of the texture
		 * @param	level specifies the mip map level of the texture to attach
		 *			to the framebuffer
		 * @param	layer specifies the layer of a 2-dimensional image within a
		 *			Texture3D or a Texture2DArray
		 * @param	orientation which face of the CubeMap is going to be set
		 *			(0 = positive X, 1 = negative X, 2 = positive Y,
		 *			3 = negative Y, 4 = positive Z, 5 = negative Z)
		 * @return	true if the texture was successfully attached, false
		 *			otherwise */
		bool attach(
			const Texture* texture,
			unsigned int attachment,
			int level = 0, int layer = 0, int orientation = 0
		);

		/** Copies the data from the given FrameBuffer to the current one
		 *
		 * @param	other a reference to the FrameBuffer to copy from
		 * @param	mask a bit mask with the FrameBuffer buffers to copy
		 * @param	x0 the X position of the current FrameBuffer to start
		 *			copying into
		 * @param	y0 the Y position of the current FrameBuffer to start
		 *			copying into
		 * @param	w0 the width to copy into the FrameBuffer
		 * @param	h0 the height to copy into the FrameBuffer
		 * @param	x1 the X position of the given FrameBuffer to start
		 *			copying from
		 * @param	y1 the Y position of the given FrameBuffer to start
		 *			copying from
		 * @param	w1 the width to copy from the FrameBuffer
		 * @param	h1 the height to copy from the FrameBuffer
		 * @param	filter the filtering method to apply (linear or nearest)
		 @return	a reference to the current FrameBuffer */
		FrameBuffer& copy(
			FrameBuffer& other, const FrameBufferMask::Mask& mask,
			std::size_t x0, std::size_t y0, std::size_t w0, std::size_t h0,
			std::size_t x1, std::size_t y1, std::size_t w1, std::size_t h1,
			TextureFilter filter
		);

		/** @copydoc Bindable::clone()
		 * @note	no Textures will be attached to the new FrameBuffer */
		virtual std::unique_ptr<Bindable> clone() const override;

		/** Binds the Frame Buffer Object */
		virtual void bind() const override;

		/** Unbinds the Frame Buffer Object */
		virtual void unbind() const override;
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
