#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H

#include <glm/glm.hpp>

typedef struct ALCdevice ALCdevice;
typedef struct ALCcontext ALCcontext;

namespace se::audio {

	/**
	 * Class AudioEngine, It's the class used to prepare the audio devices for
	 * playing sounds, and to set the properties of the Listener of the Sounds.
	 * The properties of this Listener can be used to control from where we are
	 * going to be listening the Sounds in this 3D audio scene.
	 */
	class AudioEngine
	{
	private:	// Attributes
		/** A pointer to the device that we are going to use to play Sounds. */
		ALCdevice* mDevice;

		/** A pointer to the context used to render our audio scene. */
		ALCcontext* mContext;

	public:		// Functions
		/** Creates a new AudioEngine and initializes the devices and context.
		 * The audio device will be the default one.
		 *
		 * @throw	runtime_error in case of not being able to initialize the
		 *			devices */
		AudioEngine();

		/** Class destructor. It releases the audio devices. */
		~AudioEngine();

		/** Sets the 3D position of the current Listener
		 *
		 * @param	position the new position of the Listener */
		void setListenerPosition(const glm::vec3& position) const;

		/** Sets the 3D orientation of the current Listener
		 *
		 * @param	forwardVector the vector that points to the new forward
		 *			direction of the current Listener
		 * @param	upVector the vector that points to the new up direction of
		 *			the current Listener */
		void setListenerOrientation(
			const glm::vec3& forwardVector, const glm::vec3& upVector
		) const;

		/** Sets the 3D velocity of the current Listener
		 *
		 * @param	velocity the new velocity of the Listener */
		void setListenerVelocity(const glm::vec3& velocity) const;
	};

}

#endif		// AUDIO_ENGINE_H
