#include <AL/al.h>
#include "se/audio/Source.h"
#include "se/audio/Buffer.h"
#include "se/audio/ALWrapper.h"

namespace se::audio {

	Source::Source()
	{
		AL_WRAP( alGenSources(1, &mSourceId) );
		SOMBRA_TRACE_LOG << "Created Source " << mSourceId;
	}


	Source::Source(Source&& other)
	{
		mSourceId = other.mSourceId;
		other.mSourceId = 0;
	}


	Source::~Source()
	{
		if (mSourceId != 0) {
			stop();
			AL_WRAP( alDeleteSources(1, &mSourceId) );
			SOMBRA_TRACE_LOG << "Deleted Source " << mSourceId;
		}
	}


	Source& Source::operator=(Source&& other)
	{
		if (mSourceId != 0) {
			stop();
			AL_WRAP( alDeleteSources(1, &mSourceId) );
			SOMBRA_TRACE_LOG << "Deleted Source " << mSourceId;
		}

		mSourceId = other.mSourceId;
		other.mSourceId = 0;

		return *this;
	}


	bool Source::isPlaying() const
	{
		int state = AL_INVALID_VALUE;
		AL_WRAP( alGetSourcei(mSourceId, AL_SOURCE_STATE, &state) );

		return (state == AL_PLAYING);
	}


	void Source::setPosition(const glm::vec3& position) const
	{
		AL_WRAP( alSource3f(mSourceId, AL_POSITION, position.x, position.y, position.z) );
	}


	void Source::setOrientation(
		const glm::vec3& forwardVector, const glm::vec3& upVector
	) const
	{
		float orientation[] = {
			forwardVector.x, forwardVector.y, forwardVector.z,
			upVector.x, upVector.y, upVector.z
		};
		AL_WRAP( alSourcefv(mSourceId, AL_ORIENTATION, orientation) );
	}


	void Source::setVelocity(const glm::vec3& velocity) const
	{
		AL_WRAP( alSource3f(mSourceId, AL_VELOCITY, velocity.x, velocity.y, velocity.z) );
	}


	void Source::setVolume(float volume) const
	{
		AL_WRAP( alSourcef(mSourceId, AL_GAIN, volume) );
	}


	void Source::setPitch(float pitch) const
	{
		AL_WRAP( alSourcef(mSourceId, AL_PITCH, pitch) );
	}


	void Source::setLooping(bool looping) const
	{
		AL_WRAP( alSourcei(mSourceId, AL_LOOPING, (looping)? AL_TRUE : AL_FALSE) );
	}


	void Source::bind(const Buffer& buffer) const
	{
		stop();
		AL_WRAP( alSourcei(mSourceId, AL_BUFFER, buffer.getBufferId()) );
	}


	void Source::unbind() const
	{
		stop();
		AL_WRAP( alSourcei(mSourceId, AL_BUFFER, 0) );
	}


	void Source::play() const
	{
		AL_WRAP( alSourcePlay(mSourceId) );
	}


	void Source::pause() const
	{
		AL_WRAP( alSourcePause(mSourceId) );
	}


	void Source::rewind() const
	{
		AL_WRAP( alSourceRewind(mSourceId) );
	}


	void Source::stop() const
	{
		AL_WRAP( alSourceStop(mSourceId) );
	}

}
