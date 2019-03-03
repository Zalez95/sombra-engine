#include <AL/al.h>
#include <AL/alext.h>
#include "se/audio/Buffer.h"
#include "se/audio/ALWrapper.h"

namespace se::audio {

	Buffer::Buffer()
	{
		AL_WRAP( alGenBuffers(1, &mBufferId) );
		SOMBRA_TRACE_LOG << "Created Buffer " << mBufferId;
	}


	Buffer::Buffer(Buffer&& other)
	{
		mBufferId = other.mBufferId;
		other.mBufferId = 0;
	}


	Buffer::~Buffer()
	{
		if (mBufferId != 0) {
			AL_WRAP( alDeleteBuffers(1, &mBufferId) );
			SOMBRA_TRACE_LOG << "Deleted Buffer " << mBufferId;
		}
	}


	Buffer& Buffer::operator=(Buffer&& other)
	{
		if (mBufferId != 0) {
			AL_WRAP( alDeleteBuffers(1, &mBufferId) );
		}

		mBufferId = other.mBufferId;
		other.mBufferId = 0;

		return *this;
	}


	void Buffer::setBufferFloatData(const std::vector<float>& data, int sampleRate)
	{
		AL_WRAP( alBufferData(
			mBufferId,
			AL_FORMAT_MONO_FLOAT32, data.data(), static_cast<ALsizei>(data.size() * sizeof(float)),
			sampleRate
		) );
	}

}
