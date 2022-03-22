#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include <mutex>
#include "events/ContainerEvent.h"
#include "events/EventManager.h"
#include "ECS.h"

namespace se::app {

	class Application;
	struct TransformsComponent;
	class SoundComponent;


	/**
	 * Class AudioSystem, it's a System used for updating and playing the
	 * Entities' audio Components
	 */
	class AudioSystem : public ISystem, public IEventListener
	{
	private:	// Attributes
		/** The Application that holds the AudioEngine used for playing the
		 * audio data of the Entities */
		Application& mApplication;

		/** The listener Entity*/
		Entity mListenerEntity;

		/** The mutex used for protecting @see mListenerEntity */
		std::mutex mMutex;

	public:		// Functions
		/** Creates a new AudioSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		AudioSystem(Application& application);

		/** Class destructor */
		~AudioSystem();

		/** @copydoc IEventListener::notify(const IEvent&) */
		virtual bool notify(const IEvent& event) override
		{ return tryCall(&AudioSystem::onCameraEvent, event); };

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override;

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override;

		/** Updates the sounds data with the Entities */
		virtual void update() override;
	private:
		/** Function called when a SoundComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the SoundComponent
		 * @param	sound a pointer to the new SoundComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onNewSound(
			Entity entity, SoundComponent* sound,
			EntityDatabase::Query& query
		);

		/** Function called when a SoundComponent is going to be removed
		 * from an Entity
		 *
		 * @param	entity the Entity that holds the SoundComponent
		 * @param	sound a pointer to the SoundComponent that is going
		 *			to be removed
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onRemoveSound(
			Entity entity, SoundComponent* sound,
			EntityDatabase::Query& query
		);

		/** Function called when a TransformsComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the TransformsComponent
		 * @param	transforms a pointer to the new TransformsComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onNewTransforms(
			Entity entity, TransformsComponent* transforms,
			EntityDatabase::Query& query
		);

		/** Handles the given ContainerEvent by updating the Listener Entity
		 * from where the audio Sources will be listened
		 *
		 * @param	event the ContainerEvent to handle */
		void onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event);
	};

}

#endif		// AUDIO_SYSTEM_H
