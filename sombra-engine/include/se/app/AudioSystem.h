#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include "../audio/AudioEngine.h"
#include "ISystem.h"

namespace se::app {

	/**
	 * Class AudioSystem, it's a System used for updating and playing the
	 * Entities' audio Components
	 */
	class AudioSystem : public ISystem
	{
	private:	// Attributes
		/** The AudioEngine used for playing the audio data of the Entities */
		audio::AudioEngine& mAudioEngine;

		/** The listener entity of the AudioSystem */
		Entity mListener;

	public:		// Functions
		/** Creates a new AudioSystem
		 *
		 * @param	entityDatabase the EntityDatabase that holds all the
		 *			Entities
		 * @param	audioEngine a reference to the AudioEngine used by
		 * 			the AudioSystem to play the sound sources */
		AudioSystem(
			EntityDatabase& entityDatabase, audio::AudioEngine& audioEngine
		);

		/** Class destructor */
		~AudioSystem();

		/** Function that the EntityDatabase will call when an Entity is
		 * added
		 *
		 * @param	entity the new Entity */
		virtual void onNewEntity(Entity entity);

		/** Function that the EntityDatabase will call when an Entity is
		 * removed
		 *
		 * @param	entity the Entity to remove */
		virtual void onRemoveEntity(Entity entity);

		/** Sets the given Entity as the Listener of all the audio Sources of
		 * the AudioSystem
		 *
		 * @param	entity the Entity to set as the Listener */
		void setListener(Entity entity);

		/** Updates the sources data with the Entities */
		virtual void update() override;
	};

}

#endif		// AUDIO_SYSTEM_H
