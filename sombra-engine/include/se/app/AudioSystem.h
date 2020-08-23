#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include "ISystem.h"
#include "events/ContainerEvent.h"

namespace se::app {

	class Application;


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

		/** If the listener has been updated or not */
		bool mListenerUpdated;

	public:		// Functions
		/** Creates a new AudioSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		AudioSystem(Application& application);

		/** Class destructor */
		~AudioSystem();

		/** Notifies the AudioSystem of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const IEvent& event) override;

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

		/** Updates the sources data with the Entities */
		virtual void update() override;
	private:
		/** Handles the given ContainerEvent by updating the Listener Entity
		 * from where the audio Sources will be listened
		 *
		 * @param	event the ContainerEvent to handle */
		void onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event);
	};

}

#endif		// AUDIO_SYSTEM_H
