#ifndef SOURCE_H
#define SOURCE_H

#include <glm/glm.hpp>

namespace se::audio {

	class Buffer;


	/**
	 * Class Source, a Source object represents a sound in the 3D space. It's
	 * the class used to play the data stored in the audio Buffers. It also has
	 * some properties used to control the way in which the AudioEngine will
	 * play the Sound.
	 */
	class Source
	{
	private:	// Attributes
		/** The id used to access the properties of the current source */
		unsigned int mSourceId;

	public:		// Functions
		/** Creates a new Source */
		Source();
		Source(const Source& other) = delete;
		Source(Source&& other);

		/** Class destructor */
		~Source();

		/** Assignment operator */
		Source& operator=(const Source& other) = delete;
		Source& operator=(Source&& other);

		/** @return	true if the current Source is playing some sound, false
		 *			otherwise */
		bool isPlaying() const;

		/** Sets the 3D position of the current Source
		 *
		 * @param	position the new position of the Source */
		void setPosition(const glm::vec3& position) const;

		/** Sets the 3D orientation of the current Source
		 *
		 * @param	forwardVector the vector that points to the new forward
		 *			direction of the current Source
		 * @param	upVector the vector that points to the new up direction of
		 *			the current Source */
		void setOrientation(
			const glm::vec3& forwardVector, const glm::vec3& upVector
		) const;

		/** Sets the 3D velocity of the current Source
		 *
		 * @param	velocity the new velocity of the Source */
		void setVelocity(const glm::vec3& velocity) const;

		/** Sets the volume of the sound of the current Source
		 *
		 * @param	volume the new volume of the Source */
		void setVolume(float volume) const;

		/** Sets the pitch of the sound of the current Source
		 *
		 * @param	pitch the new pitch of the Source */
		void setPitch(float pitch) const;

		/** Sets the looping property of the current Source
		 *
		 * @param	looping true if we want to keep repeating the sound of the
		 *			Source, false otherwise */
		void setLooping(bool looping) const;

		/** Binds the given Buffer to the current Source, so the next Sound
		 * that the Source will play will be the one stored in the given
		 * buffer
		 *
		 * @param	buffer the buffer to bind to the current Source
		 * @note	this function will also stop the Source from playing the
		 *			Sound its old of its old Buffer */
		void bind(const Buffer& buffer) const;

		/** Stops and unbinds the Source from its current Buffer */
		void unbind() const;

		/** Starts playing the sound of its binded Buffer
		 *
		 * @note	if the Source was paused the function will resume Sound in
		 *			the same point that it was paused */
		void play() const;

		/** Pauses a Source from playing the Sound of its binded Buffer */
		void pause() const;

		/** Rewinds a Source so the next time we call the play function the
		 * Sound of its binded Buffer will start playing like the first time */
		void rewind() const;

		/** Stops a Source from playing the sound of its binded Buffer */
		void stop() const;
	};

}

#endif		// SOURCE_H
