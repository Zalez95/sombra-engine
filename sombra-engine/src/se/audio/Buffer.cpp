#include <AL/al.h>
#include <AL/alext.h>
#include "se/audio/Buffer.h"

namespace se::audio {

	Buffer::Buffer()
	{
		alGenBuffers(1, &mBufferId);
	}


	Buffer::~Buffer()
	{
		alDeleteBuffers(1, &mBufferId);
	}


	void Buffer::setBufferFloatData(const std::vector<float>& data, int sampleRate)
	{
		alBufferData(
			mBufferId,
			AL_FORMAT_MONO_FLOAT32, data.data(), data.size() * sizeof(float),
			sampleRate
		);
	}

}
