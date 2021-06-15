#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include "ECS.h"
#include "events/ContainerEvent.h"

namespace se::app {

	class Application;
	class AudioSourceComponent;


	/**
	 * Class AudioSystem, it's a System used for updating and playing the
	 * Entities' audio Components
	 */
	class AudioSystem : public ISystem
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

		/** @copydoc ISystem::notify(const IEvent&) */
		virtual void notify(const IEvent& event) override
		{ tryCall(&AudioSystem::onCameraEvent, event); };

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&AudioSystem::onNewSource, entity, mask); };

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&AudioSystem::onRemoveSource, entity, mask); };

		/** Updates the sources data with the Entities */
		virtual void update() override;
	private:
		/** Function called when a AudioSourceComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the AudioSourceComponent
		 * @param	source a pointer to the new AudioSourceComponent */
		void onNewSource(Entity entity, AudioSourceComponent* source);

		/** Function called when a AudioSourceComponent is going to be removed
		 * from an Entity
		 *
		 * @param	entity the Entity that holds the AudioSourceComponent
		 * @param	source a pointer to the AudioSourceComponent that is going
		 *			to be removed */
		void onRemoveSource(Entity entity, AudioSourceComponent* source);

		/** Handles the given ContainerEvent by updating the Listener Entity
		 * from where the audio Sources will be listened
		 *
		 * @param	event the ContainerEvent to handle */
		void onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event);
	};

}

#endif		// AUDIO_SYSTEM_H
