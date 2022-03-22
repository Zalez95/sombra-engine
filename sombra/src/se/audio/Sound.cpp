#include <miniaudio.h>
#include <se/utils/Log.h>
#include "se/audio/Sound.h"
#include "se/audio/DataSource.h"
#include "se/audio/AudioEngine.h"

namespace se::audio {

	Sound::Sound() {}


	Sound::Sound(const Sound& other)
	{
		*this = other;
	}


	Sound::Sound(Sound&& other) : mSound(std::move(other.mSound)) {}


	Sound::~Sound()
	{
		if (mSound) {
			ma_sound_uninit(mSound.get());
			SOMBRA_TRACE_LOG << "Deleted Sound " << mSound.get();
			mSound = nullptr;
		}
	}


	Sound& Sound::operator=(const Sound& other)
	{
		if (mSound) {
			ma_sound_uninit(mSound.get());
			SOMBRA_TRACE_LOG << "Deleted Sound " << mSound.get();
			mSound = nullptr;
		}

		mSound = std::make_unique<ma_sound>();
		ma_engine* engine = ma_sound_get_engine(other.mSound.get());
		ma_result res = ma_sound_init_copy(engine, other.mSound.get(), 0, nullptr, mSound.get());
		if (res != MA_SUCCESS) {
			SOMBRA_ERROR_LOG << "Failed to create the sound";
			mSound = nullptr;
			return *this;
		}

		SOMBRA_TRACE_LOG << "Created Sound " << mSound.get();

		return *this;
	}


	Sound& Sound::operator=(Sound&& other)
	{
		if (mSound) {
			ma_sound_uninit(mSound.get());
			SOMBRA_TRACE_LOG << "Deleted Sound " << mSound.get();
			mSound = nullptr;
		}

		mSound = std::move(other.mSound);

		return *this;
	}


	bool Sound::init(AudioEngine& audioEngine)
	{
		if (!mSound) {
			mSound = std::make_unique<ma_sound>();

			ma_sound_config soundConfig;
			soundConfig = ma_sound_config_init();

			ma_result res = ma_sound_init_ex(audioEngine.mEngine.get(), &soundConfig, mSound.get());
			if (res != MA_SUCCESS) {
				SOMBRA_ERROR_LOG << "Failed to create sound";
				mSound = nullptr;
				return false;
			}

			SOMBRA_TRACE_LOG << "Created Sound " << mSound.get();
		}

		return true;
	}


	bool Sound::isPlaying() const
	{
		return ma_sound_is_playing(mSound.get());
	}


	Sound& Sound::setSpacialization(bool value)
	{
		ma_sound_set_spatialization_enabled(mSound.get(), value);
		return *this;
	}


	glm::vec3 Sound::getPosition() const
	{
		ma_vec3f pos = ma_sound_get_position(mSound.get());
		return { pos.x, pos.y, pos.z };
	}


	Sound& Sound::setPosition(const glm::vec3& position)
	{
		ma_sound_set_position(mSound.get(), position.x, position.y, position.z);
		return *this;
	}


	glm::vec3 Sound::getOrientation() const
	{
		ma_vec3f dir = ma_sound_get_direction(mSound.get());
		return { dir.x, dir.y, dir.z };
	}


	Sound& Sound::setOrientation(const glm::vec3& forwardVector)
	{
		ma_sound_set_direction(mSound.get(), forwardVector.x, forwardVector.y, forwardVector.z);
		return *this;
	}


	void Sound::getSoundCone(float& innerAngle, float& outerAngle, float& outerGain) const
	{
		ma_sound_get_cone(mSound.get(), &innerAngle, &outerAngle, &outerGain);
	}


	Sound& Sound::setSoundCone(float innerAngle, float outerAngle, float outerGain)
	{
		ma_sound_set_cone(mSound.get(), innerAngle, outerAngle, outerGain);
		return *this;
	}


	glm::vec3 Sound::getVelocity() const
	{
		ma_vec3f vel = ma_sound_get_velocity(mSound.get());
		return { vel.x, vel.y, vel.z };
	}


	Sound& Sound::setVelocity(const glm::vec3& velocity)
	{
		ma_sound_set_velocity(mSound.get(), velocity.x, velocity.y, velocity.z);
		return *this;
	}


	float Sound::getVolume() const
	{
		return ma_sound_get_volume(mSound.get());
	}


	Sound& Sound::setVolume(float volume)
	{
		ma_sound_set_volume(mSound.get(), volume);
		return *this;
	}


	float Sound::getPitch() const
	{
		return ma_sound_get_pitch(mSound.get());
	}


	Sound& Sound::setPitch(float pitch)
	{
		ma_sound_set_pitch(mSound.get(), pitch);
		return *this;
	}


	bool Sound::isLooping() const
	{
		return ma_sound_is_looping(mSound.get());
	}


	Sound& Sound::setLooping(bool looping)
	{
		ma_sound_set_looping(mSound.get(), looping);
		return *this;
	}


	void Sound::bind(const DataSource& source)
	{
		ma_engine* engine = ma_sound_get_engine(source.mDataSourceOwner.get());
		ma_data_source* dataSource = ma_sound_get_data_source(source.mDataSourceOwner.get());

		Sound other;
		other.mSound = std::make_unique<ma_sound>();
		ma_result res = ma_sound_init_from_data_source(engine, dataSource, 0, nullptr, other.mSound.get());
		if (res != MA_SUCCESS) {
			SOMBRA_ERROR_LOG << "Failed to create the sound";
			return;
		}

		SOMBRA_TRACE_LOG << "Created Sound " << mSound.get();

		other.setPosition( getPosition() );
		other.setOrientation( getOrientation() );
		float innerAngle, outerAngle, outerGain;
		getSoundCone(innerAngle, outerAngle, outerGain);
		other.setSoundCone(innerAngle, outerAngle, outerGain);
		other.setVelocity( getVelocity() );
		other.setVolume( getVolume() );
		other.setPitch( getPitch() );
		other.setLooping( isLooping() );

		*this = std::move(other);
	}


	void Sound::unbind()
	{
		DataSource nullSource;
		bind(nullSource);
	}


	void Sound::play() const
	{
		ma_sound_start(mSound.get());
	}


	void Sound::pause() const
	{
		ma_sound_stop(mSound.get());
	}


	void Sound::setToPCMFrame(unsigned int frame) const
	{
		ma_sound_seek_to_pcm_frame(mSound.get(), frame);
	}


	void Sound::stop() const
	{
		pause();
		setToPCMFrame(0);
	}

}
