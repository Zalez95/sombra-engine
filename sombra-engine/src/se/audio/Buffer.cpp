#include <AL/al.h>
#include <AL/alext.h>
#include "se/audio/Buffer.h"
#include "se/audio/ALWrapper.h"

namespace se::audio {

	constexpr ALenum toALFormat(FormatId format)
	{
		switch (format) {
			case FormatId::Mono8:			return AL_FORMAT_MONO8;
			case FormatId::Mono16:			return AL_FORMAT_MONO16;
			case FormatId::MonoFloat:		return AL_FORMAT_MONO_FLOAT32;
			case FormatId::MonoDouble:		return AL_FORMAT_MONO_DOUBLE_EXT;
			case FormatId::Stereo8:			return AL_FORMAT_STEREO8;
			case FormatId::Stereo16:		return AL_FORMAT_STEREO16;
			case FormatId::StereoFloat:		return AL_FORMAT_STEREO_FLOAT32;
			case FormatId::StereoDouble:	return AL_FORMAT_STEREO_DOUBLE_EXT;
			default:						return AL_NONE;
		}
	}


	Buffer::Buffer(const void* data, std::size_t size, FormatId format, int sampleRate)
	{
		AL_WRAP( alGenBuffers(1, &mBufferId) );
		AL_WRAP( alBufferData(
			mBufferId,
			toALFormat(format),
			data, static_cast<ALsizei>(size),
			sampleRate
		) );

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
			SOMBRA_TRACE_LOG << "Deleted Buffer " << mBufferId;
		}

		mBufferId = other.mBufferId;
		other.mBufferId = 0;

		return *this;
	}

}
