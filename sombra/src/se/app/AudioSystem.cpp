#include "se/utils/Log.h"
#include "se/audio/AudioEngine.h"
#include "se/app/AudioSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/AudioSourceComponent.h"

namespace se::app {

	AudioSystem::AudioSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application), mListenerEntity(kNullEntity)
	{
		mApplication.getEventManager().subscribe(this, Topic::Camera);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<AudioSourceComponent>());
	}


	AudioSystem::~AudioSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Camera);
	}


	void AudioSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the AudioSystem";

		SOMBRA_DEBUG_LOG << "Updating the Listener";
		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(mListenerEntity, true);
		if (transforms) {
			auto& audioEngine = *mApplication.getExternalTools().audioEngine;
			audioEngine.setListenerPosition(transforms->position);
			audioEngine.setListenerOrientation(
				glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation,
				glm::vec3(0.0f, 1.0f, 0.0)
			);
			audioEngine.setListenerVelocity(transforms->velocity);
		}

		SOMBRA_DEBUG_LOG << "Updating the Sources";
		mEntityDatabase.iterateComponents<TransformsComponent, AudioSourceComponent>(
			[this](Entity, TransformsComponent* transforms, AudioSourceComponent* source) {
				if (!transforms->updated[static_cast<int>(TransformsComponent::Update::AudioSource)]) {
					source->get().setPosition(transforms->position);
					source->get().setOrientation(
						glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation,
						glm::vec3(0.0f, 1.0f, 0.0)
					);
					source->get().setVelocity(transforms->velocity);

					transforms->updated.set(static_cast<int>(TransformsComponent::Update::AudioSource));
				}
			},
			true
		);

		SOMBRA_DEBUG_LOG << "AudioSystem updated";
	}

// Private functions
	void AudioSystem::onNewSource(Entity entity, AudioSourceComponent* source)
	{
		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(entity, true);
		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::AudioSource));
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with AudioSourceComponent " << source << " added successfully";
	}


	void AudioSystem::onRemoveSource(Entity entity, AudioSourceComponent* source)
	{
		SOMBRA_INFO_LOG << "Entity " << entity << " with AudioSourceComponent " << source << " removed successfully";
	}


	void AudioSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		SOMBRA_INFO_LOG << event;
		mListenerEntity = event.getValue();
	}

}
