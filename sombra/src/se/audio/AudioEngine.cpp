#include <stdexcept>
#include <AL/al.h>
#include <AL/alc.h>
#include "se/audio/AudioEngine.h"
#include "se/audio/ALWrapper.h"
#include "se/audio/ALCWrapper.h"

namespace se::audio {

	AudioEngine::AudioEngine()
	{
		mDevice = alcOpenDevice(nullptr);
		if (!mDevice) {
			throw std::runtime_error("Can't open the Device");
		}

		ALC_WRAP( mContext = alcCreateContext(mDevice, nullptr), mDevice );

		ALC_WRAP( bool currentContext = alcMakeContextCurrent(mContext), mDevice );
		if (!currentContext) {
			alcCloseDevice(mDevice);
			throw std::runtime_error("Context creation error");
		}
	}


	AudioEngine::~AudioEngine()
	{
		ALC_WRAP( alcMakeContextCurrent(nullptr), mDevice );
		ALC_WRAP( alcDestroyContext(mContext), mDevice );
		alcCloseDevice(mDevice);
	}


	void AudioEngine::setListenerPosition(const glm::vec3& position) const
	{
		AL_WRAP( alListener3f(AL_POSITION, position.x, position.y, position.z) );
	}


	void AudioEngine::setListenerOrientation(
		const glm::vec3& forwardVector, const glm::vec3& upVector
	) const
	{
		float orientation[] = {
			forwardVector.x, forwardVector.y, forwardVector.z,
			upVector.x, upVector.y, upVector.z
		};
		AL_WRAP( alListenerfv(AL_ORIENTATION, orientation) );
	}


	void AudioEngine::setListenerVelocity(const glm::vec3& velocity) const
	{
		AL_WRAP( alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z) );
	}

}
