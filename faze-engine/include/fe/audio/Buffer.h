#ifndef BUFFER_H
#define BUFFER_H

#include <vector>

namespace fe { namespace audio {

	/**
	 * Class Buffer, a buffer is an object used to create, delete and access to
	 * the buffered audio data. A buffer can be shared between multiple audio
	 * sources to reduce memory consumption.
	 */
	class Buffer
	{
	private:	// Attributes
		friend class Source;

		/** The index used to access to the audio data */
		unsigned int mBufferId;

	public:		// Functions
		/** Creates a new Buffer */
		Buffer();

		/** Class destructor */
		~Buffer();

		/** Sets the given raw float data as the audio data of the current
		 * buffer
		 *
		 * @param	data the audio data to store in the buffer
		 * @param	sampleRate the sampling rate of the given audio data */
		void setBufferFloatData(const std::vector<float>& data, int sampleRate);
	};

}}

#endif		// BUFFER_H
