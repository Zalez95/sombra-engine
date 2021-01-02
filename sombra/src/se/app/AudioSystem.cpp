#include "se/utils/Log.h"
#include "se/audio/Source.h"
#include "se/audio/AudioEngine.h"
#include "se/app/AudioSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	AudioSystem::AudioSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application),
		mListenerEntity(kNullEntity), mListenerUpdated(false)
	{
		mApplication.getEventManager().subscribe(this, Topic::Camera);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<audio::Source>());
	}


	AudioSystem::~AudioSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Camera);
	}


	void AudioSystem::notify(const IEvent& event)
	{
		tryCall(&AudioSystem::onCameraEvent, event);
	}


	void AudioSystem::onNewEntity(Entity entity)
	{
		auto [transforms, source] = mEntityDatabase.getComponents<TransformsComponent, audio::Source>(entity);
		if (!source) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as Source";
			return;
		}

		if (transforms) {
			// The Source initial data is overridden by the entity one
			source->setPosition(transforms->position);
			source->setOrientation(glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation, glm::vec3(0.0f, 1.0f, 0.0));
			source->setVelocity(transforms->velocity);
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with Source " << source << " added successfully";
	}


	void AudioSystem::onRemoveEntity(Entity entity)
	{
		if (mListenerEntity == entity) {
			mListenerEntity = kNullEntity;
			mListenerUpdated = true;
			SOMBRA_INFO_LOG << "Listener Entity " << entity << " removed successfully";
		}
	}


	void AudioSystem::update()
	{
		SOMBRA_INFO_LOG << "Updating the AudioSystem";

		SOMBRA_DEBUG_LOG << "Updating the Listener";
		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(mListenerEntity);
		if (transforms && (transforms->updated.any() || mListenerUpdated)) {
			auto& audioEngine = *mApplication.getExternalTools().audioEngine;
			audioEngine.setListenerPosition(transforms->position);
			audioEngine.setListenerOrientation(
				glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation,
				glm::vec3(0.0f, 1.0f, 0.0)
			);
			audioEngine.setListenerVelocity(transforms->velocity);
		}
		mListenerUpdated = false;

		SOMBRA_DEBUG_LOG << "Updating the Sources";
		mEntityDatabase.iterateComponents<TransformsComponent, audio::Source>(
			[this](Entity, TransformsComponent* transforms, audio::Source* source) {
				if (transforms->updated.any()) {
					source->setPosition(transforms->position);
					source->setOrientation(
						glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation,
						glm::vec3(0.0f, 1.0f, 0.0)
					);
					source->setVelocity(transforms->velocity);
				}
			}
		);

		SOMBRA_INFO_LOG << "AudioSystem updated";
	}

// Private functions
	void AudioSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		mListenerEntity = event.getValue();
		mListenerUpdated = true;
	}

}