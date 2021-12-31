#ifndef BUFFER_H
#define BUFFER_H

#include <vector>

namespace se::audio {

	/**
	 * The different data formats that can be stored in a Buffer
	 */
 	enum class FormatId : int
	{
		Mono8,			Stereo8,
		Mono16,			Stereo16,
		MonoFloat,		StereoFloat,
		MonoDouble,		StereoDouble,
	};


	/**
	 * Class Buffer, a buffer is an object used to create, delete and access to
	 * the buffered audio data. A buffer can be shared between multiple audio
	 * sources to reduce memory consumption.
	 */
	class Buffer
	{
	private:	// Attributes
		/** The index used to access to the audio data */
		unsigned int mBufferId;

	public:		// Functions
		/** Creates a new Buffer */
		Buffer();

		/** Creates a new Buffer */
		Buffer(const Buffer& other) = delete;
		Buffer(Buffer&& other);

		/** Class destructor */
		~Buffer();

		/** Assignment operator */
		Buffer& operator=(const Buffer& other) = delete;
		Buffer& operator=(Buffer&& other);

		/** @return the id of the Buffer */
		inline unsigned int getBufferId() const { return mBufferId; };

		/** Sets the buffer data
		 *
		 * @param	data a pointer to the data of the buffer
		 * @param	size the size of the data buffer
		 * @param	format the FormatId of the data stored in the Buffer
		 * @param	sampleRate the sampling rate of the given audio data
		 * @return	a reference to the current Buffer object */
		Buffer& setData(
			const void* data, std::size_t size,
			FormatId format, int sampleRate
		);
	};

}

#endif		// BUFFER_H
