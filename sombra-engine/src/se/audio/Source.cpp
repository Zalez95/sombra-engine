#include <AL/al.h>
#include "se/audio/Source.h"
#include "se/audio/Buffer.h"

namespace se::audio {

	Source::Source()
	{
		alGenSources(1, &mSourceId);
	}


	Source::~Source()
	{
		stop();
		alDeleteSources(1, &mSourceId);
	}


	bool Source::isPlaying() const
	{
		int state;
		alGetSourcei(mSourceId, AL_SOURCE_STATE, &state);

		return (state == AL_PLAYING);
	}


	void Source::setPosition(const glm::vec3& position) const
	{
		alSource3f(mSourceId, AL_POSITION, position.x, position.y, position.z);
	}


	void Source::setOrientation(
		const glm::vec3& forwardVector, const glm::vec3& upVector
	) const
	{
		float orientation[] = {
			forwardVector.x, forwardVector.y, forwardVector.z,
			upVector.x, upVector.y, upVector.z
		};
		alSourcefv(mSourceId, AL_ORIENTATION, orientation);
	}


	void Source::setVelocity(const glm::vec3& velocity) const
	{
		alSource3f(mSourceId, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
	}


	void Source::setVolume(float volume) const
	{
		alSourcef(mSourceId, AL_GAIN, volume);
	}


	void Source::setPitch(float pitch) const
	{
		alSourcef(mSourceId, AL_PITCH, pitch);
	}


	void Source::setLooping(bool looping) const
	{
		alSourcei(mSourceId, AL_LOOPING, (looping)? AL_TRUE : AL_FALSE);
	}


	void Source::bind(const Buffer& buffer) const
	{
		stop();
		alSourcei(mSourceId, AL_BUFFER, buffer.mBufferId);
	}


	void Source::unbind() const
	{
		stop();
		alSourcei(mSourceId, AL_BUFFER, 0);
	}


	void Source::play() const
	{
		alSourcePlay(mSourceId);
	}


	void Source::pause() const
	{
		alSourcePause(mSourceId);
	}


	void Source::rewind() const
	{
		alSourceRewind(mSourceId);
	}


	void Source::stop() const
	{
		alSourceStop(mSourceId);
	}

}
