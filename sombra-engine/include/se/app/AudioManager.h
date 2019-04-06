#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <map>
#include <memory>
#include "../audio/AudioEngine.h"
#include "../audio/Source.h"

namespace se::app {

	struct Entity;


	/**
	 * Class AudioManager, it's a Manager used for storing, updating and
	 * playing the Entities' audio data
	 */
	class AudioManager
	{
	private:	// Nested types
		using SourceUPtr = std::unique_ptr<audio::Source>;

	private:	// Attributes
		/** The AudioEngine used for playing the audio data of the Entities */
		audio::AudioEngine& mAudioEngine;

		/** All the Source Entities added to the AudioManager */
		std::map<Entity*, SourceUPtr> mSourceEntities;

		/** The listener entity of the AudioManager */
		Entity* mListener;

	public:		// Functions
		/** Creates a new AudioManager
		 *
		 * @param	audioEngine a reference to the AudioEngine used by
		 * 			the AudioManager to play the sound sources */
		AudioManager(audio::AudioEngine& audioEngine) :
			mAudioEngine(audioEngine) {};

		/** Sets the given Entity as the Listener of all the audio Sources of
		 * the AudioManager
		 *
		 * @param	entity a pointer to the Entity to set as the Listener */
		void setListener(Entity* entity);

		/** Adds the given Entity as an audio Source to the AudioManager
		 *
		 * @param	entity a pointer to the Entity to add to the
		 *			AudioManager
		 * @param	source a pointer to the Source to add to the
		 *			AudioManager */
		void addSource(Entity* entity, SourceUPtr source);

		/** Removes the given Entity from the AudioManager so it won't
		 * longer be played
		 *
		 * @param	entity a pointer to the Entity to remove from the
		 *			AudioManager */
		void removeEntity(Entity* entity);

		/** Updates the sources data with the Entities */
		void update();
	};

}

#endif		// AUDIO_MANAGER_H
