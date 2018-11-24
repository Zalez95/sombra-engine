#include <AL/al.h>
#include <AL/alext.h>
#include "se/audio/Buffer.h"
#include "se/audio/ALWrapper.h"

namespace se::audio {

	Buffer::Buffer()
	{
		AL_WRAP( alGenBuffers(1, &mBufferId) );
	}


	Buffer::~Buffer()
	{
		AL_WRAP( alDeleteBuffers(1, &mBufferId) );
	}


	void Buffer::setBufferFloatData(const std::vector<float>& data, int sampleRate)
	{
		AL_WRAP(
			alBufferData(
				mBufferId,
				AL_FORMAT_MONO_FLOAT32, data.data(), data.size() * sizeof(float),
				sampleRate
			)
		);
	}

}
