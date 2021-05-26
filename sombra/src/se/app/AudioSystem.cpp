#include "se/utils/Log.h"
#include "se/audio/Source.h"
#include "se/audio/AudioEngine.h"
#include "se/app/AudioSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	AudioSystem::AudioSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application), mListenerEntity(kNullEntity)
	{
		mApplication.getEventManager().subscribe(this, Topic::Camera);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<audio::Source>());
	}


	AudioSystem::~AudioSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Camera);
	}


	void AudioSystem::update()
	{
		SOMBRA_INFO_LOG << "Updating the AudioSystem";

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
		mEntityDatabase.iterateComponents<TransformsComponent, audio::Source>(
			[this](Entity, TransformsComponent* transforms, audio::Source* source) {
				if (!transforms->updated[static_cast<int>(TransformsComponent::Update::Source)]) {
					source->setPosition(transforms->position);
					source->setOrientation(
						glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation,
						glm::vec3(0.0f, 1.0f, 0.0)
					);
					source->setVelocity(transforms->velocity);

					transforms->updated.set(static_cast<int>(TransformsComponent::Update::Source));
				}
			},
			true
		);

		SOMBRA_INFO_LOG << "AudioSystem updated";
	}

// Private functions
	void AudioSystem::onNewSource(Entity entity, audio::Source* source)
	{
		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(entity, true);
		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Source));
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with Source " << source << " added successfully";
	}


	void AudioSystem::onRemoveSource(Entity entity, audio::Source* source)
	{
		SOMBRA_INFO_LOG << "Entity " << entity << " with Source " << source << " removed successfully";
	}


	void AudioSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		mListenerEntity = event.getValue();
	}

}
