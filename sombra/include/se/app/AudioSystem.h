#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include "events/ContainerEvent.h"
#include "events/EventManager.h"
#include "ECS.h"

namespace se::app {

	class Application;
	class AudioSourceComponent;


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
		) override
		{ tryCallC(&AudioSystem::onNewSource, entity, mask, query); };

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override
		{ tryCallC(&AudioSystem::onRemoveSource, entity, mask, query); };

		/** Updates the sources data with the Entities */
		virtual void update() override;
	private:
		/** Function called when a AudioSourceComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the AudioSourceComponent
		 * @param	source a pointer to the new AudioSourceComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onNewSource(
			Entity entity, AudioSourceComponent* source,
			EntityDatabase::Query& query
		);

		/** Function called when a AudioSourceComponent is going to be removed
		 * from an Entity
		 *
		 * @param	entity the Entity that holds the AudioSourceComponent
		 * @param	source a pointer to the AudioSourceComponent that is going
		 *			to be removed
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onRemoveSource(
			Entity entity, AudioSourceComponent* source,
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
