#include <stdexcept>
#include <AL/al.h>
#include <AL/alc.h>
#include "se/audio/AudioEngine.h"

namespace se::audio {

	AudioEngine::AudioEngine()
	{
		mDevice = alcOpenDevice(nullptr);
		if (!mDevice) {
			throw std::runtime_error("Can't open the Device");
		}

		mContext = alcCreateContext(mDevice, nullptr);
		if (!alcMakeContextCurrent(mContext)) {
			alcCloseDevice(mDevice);
			throw std::runtime_error("Context creation error");
		}
	}


	AudioEngine::~AudioEngine()
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(mContext);
		alcCloseDevice(mDevice);
	}


	void AudioEngine::setListenerPosition(const glm::vec3& position) const
	{
		alListener3f(AL_POSITION, position.x, position.y, position.z);
	}


	void AudioEngine::setListenerOrientation(
		const glm::vec3& forwardVector, const glm::vec3& upVector
	) const
	{
		float orientation[] = {
			forwardVector.x, forwardVector.y, forwardVector.z,
			upVector.x, upVector.y, upVector.z
		};
		alListenerfv(AL_ORIENTATION, orientation);
	}


	void AudioEngine::setListenerVelocity(const glm::vec3& velocity) const
	{
		alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
	}

}
