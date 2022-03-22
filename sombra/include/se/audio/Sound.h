#ifndef SOUND_H
#define SOUND_H

#include <memory>
#include <glm/glm.hpp>

struct ma_sound;

namespace se::audio {

	class DataSource;
	class AudioEngine;


	/**
	 * Class Sound, a Sound object represents a sound in the 3D space. It's
	 * the class used to play the data stored in the audio Buffers. It also has
	 * some properties used to control the way in which the AudioEngine will
	 * play the Sound.
	 */
	class Sound
	{
	private:	// Attributes
		/** A pointer to the Sound object */
		std::unique_ptr<ma_sound> mSound;

	public:		// Functions
		/** Creates a new Sound */
		Sound();
		Sound(const Sound& other);
		Sound(Sound&& other);

		/** Class destructor */
		~Sound();

		/** Assignment operator */
		Sound& operator=(const Sound& other);
		Sound& operator=(Sound&& other);

		/** Initializes the Sound
		 *
		 * @param	audioEngine the AudioEngine that holds the Sound
		 * @return	true if the Sound was initialized successfully, false
		 *			otherwise */
		bool init(AudioEngine& audioEngine);

		/** @return	true if the current Sound is playing some sound, false
		 *			otherwise */
		bool isPlaying() const;

		/** Enables/disables 3D spacialization
		 *
		 * @param	value true for enabling spacilization (enabled by default),
		 *			false for disabling spacialization
		 * @return	a reference to the current Sound object */
		Sound& setSpacialization(bool value);

		/** @return	the 3D position of the current Sound */
		glm::vec3 getPosition() const;

		/** Sets the 3D position of the current Sound
		 *
		 * @param	position the new position of the Sound
		 * @return	a reference to the current Sound object */
		Sound& setPosition(const glm::vec3& position);

		/** @return	the 3D orientation of the current Sound (forward vector) */
		glm::vec3 getOrientation() const;

		/** Sets the 3D orientation of the current Sound
		 *
		 * @param	forwardVector the vector that points to the new forward
		 *			direction of the current Sound
		 * @return	a reference to the current Sound object */
		Sound& setOrientation(const glm::vec3& forwardVector);

		/** Returns the directional attenuation of the Sound
		 *
		 * @param	innerAngle a reference to the float where the angle in
		 *			radians where a listener inside that cone will have no
		 *			attenuation will be stored
		 * @param	outerAngle a reference to the float where the angle in
		 *			radians where a listener outside that cone will have an
		 *			attenuation set to @see outerGain will be stored.
		 * @param	outerGain a reference to the float where the gain used for
		 *			the @see outerAngle cone will be stored */
		void getSoundCone(
			float& innerAngle, float& outerAngle, float& outerGain
		) const;

		/** Sets the directional attenuation of the Sound
		 *
		 * @param	innerAngle the angle in radians where a listener inside that
		 *			cone will have no attenuation
		 * @param	outerAngle the angle in radians where a listener outside
		 *			that cone will have an attenuation set to @see outerGain.
		 *			The sounds inside the outerAngle cone and outside
		 *			@see innerAngle cone will be interpolated between 1 and
		 *			@see outerGain
		 * @param	outerGain the gain used for the @see outerAngle cone
		 * @return	a reference to the current Sound object */
		Sound& setSoundCone(
			float innerAngle, float outerAngle, float outerGain
		);

		/** @return	the 3D velocity of the current Sound */
		glm::vec3 getVelocity() const;

		/** Sets the 3D velocity of the current Sound
		 *
		 * @param	velocity the new velocity of the Sound
		 * @return	a reference to the current Sound object */
		Sound& setVelocity(const glm::vec3& velocity);

		/** @return	the volume of the current Sound */
		float getVolume() const;

		/** Sets the volume of the current Sound
		 *
		 * @param	volume the new volume of the Sound
		 * @return	a reference to the current Sound object */
		Sound& setVolume(float volume);

		/** @return	the pitch of the current Sound */
		float getPitch() const;

		/** Sets the pitch of the current Sound
		 *
		 * @param	pitch the new pitch of the Sound
		 * @return	a reference to the current Sound object */
		Sound& setPitch(float pitch);

		/** @return	the looping property of the current Sound */
		bool isLooping() const;

		/** Sets the looping property of the current Sound
		 *
		 * @param	looping true if we want to keep repeating the sound of the
		 *			Sound, false otherwise
		 * @return	a reference to the current Sound object */
		Sound& setLooping(bool looping);

		/** Binds the given DataSource to the current Sound, so the next sound
		 * that the Sound will play will be the one stored in the given
		 * DataSource
		 *
		 * @param	source the DataSource to bind to the current Sound
		 * @note	this function will also stop the Sound from playing the
		 *			Sound its old of its old Buffer */
		void bind(const DataSource& source);

		/** Stops and unbinds the Sound from its current Buffer */
		void unbind();

		/** Starts playing the sound of its binded Buffer
		 *
		 * @note	if the Sound was paused the function will resume Sound in
		 *			the same point that it was paused */
		void play() const;

		/** Pauses a Sound from playing the Sound of its binded Buffer */
		void pause() const;

		/** Moves a Sound to the given frame
		 *
		 * @param	frame the new frame to play */
		void setToPCMFrame(unsigned int frame) const;

		/** Stops a Sound from playing the sound of its binded Buffer with
		 * @see pause and @see setToPCMFrame(0) */
		void stop() const;
	};

}

#endif		// SOUND_H
