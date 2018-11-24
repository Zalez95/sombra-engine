#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

namespace se::graphics {

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

		/** Binds the Frame Buffer Object for reading data from it */
		void bindForReading() const;

		/** Binds the Frame Buffer Object for writing data to it */
		void bindForWriting() const;
	};

}

#endif		// FRAME_BUFFER_H
