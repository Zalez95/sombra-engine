#include <iostream>
#include <AL/al.h>
#include <AL/alc.h>
#include "fe/audio/AudioEngine.h"

namespace fe { namespace audio {

	AudioEngine::AudioEngine()
	{
		mDevice = alcOpenDevice(nullptr);
		if (!mDevice) {
			std::cerr << "device error" << std::endl;
			return;
		}

		mContext = alcCreateContext(mDevice, nullptr);
		if (!alcMakeContextCurrent(mContext)) {
			std::cerr << "context error" << std::endl;
			return;
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

}}
