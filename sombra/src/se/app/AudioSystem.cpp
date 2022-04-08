#include "se/utils/Log.h"
#include "se/audio/AudioEngine.h"
#include "se/app/AudioSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/SoundComponent.h"

namespace se::app {

	AudioSystem::AudioSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application), mListenerEntity(kNullEntity)
	{
		mApplication.getEventManager().subscribe(this, Topic::Camera);
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<SoundComponent>()
			.set<TransformsComponent>()
		);
	}


	AudioSystem::~AudioSystem()
	{
		mEntityDatabase.removeSystem(this);
		mApplication.getEventManager().unsubscribe(this, Topic::Camera);
	}


	void AudioSystem::onNewComponent(
		Entity entity, const EntityDatabase::ComponentMask& mask,
		EntityDatabase::Query& query
	) {
		tryCallC(&AudioSystem::onNewSound, entity, mask, query);
		tryCallC(&AudioSystem::onNewTransforms, entity, mask, query);
	}


	void AudioSystem::onRemoveComponent(
		Entity entity, const EntityDatabase::ComponentMask& mask,
		EntityDatabase::Query& query
	) {
		tryCallC(&AudioSystem::onRemoveSound, entity, mask, query);
	}


	void AudioSystem::update(float, float)
	{
		SOMBRA_DEBUG_LOG << "Updating the AudioSystem";

		SOMBRA_DEBUG_LOG << "Updating the Listener";
		mEntityDatabase.executeQuery([this](EntityDatabase::Query& query) {
			std::scoped_lock lock(mMutex);
			auto [transforms] = query.getComponents<TransformsComponent>(mListenerEntity, true);
			if (transforms) {
				auto& audioEngine = *mApplication.getExternalTools().audioEngine;
				audioEngine.setListenerPosition(transforms->position);
				audioEngine.setListenerOrientation(
					glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation,
					glm::vec3(0.0f, 1.0f, 0.0)
				);
				audioEngine.setListenerVelocity(transforms->velocity);
			}
		});

		SOMBRA_DEBUG_LOG << "Updating the Sources";
		mEntityDatabase.executeQuery([this](EntityDatabase::Query& query) {
			query.iterateEntityComponents<TransformsComponent, SoundComponent>(
				[this](Entity, TransformsComponent* transforms, SoundComponent* sound) {
					if (!transforms->updated[static_cast<int>(TransformsComponent::Update::AudioSource)]) {
						sound->get().setPosition(transforms->position);
						sound->get().setOrientation(glm::vec3(0.0f, 0.0f, 1.0f) * transforms->orientation);
						sound->get().setVelocity(transforms->velocity);

						transforms->updated.set(static_cast<int>(TransformsComponent::Update::AudioSource));
					}
				},
				true
			);
		});

		SOMBRA_DEBUG_LOG << "AudioSystem updated";
	}

// Private functions
	void AudioSystem::onNewSound(Entity entity, SoundComponent* sound, EntityDatabase::Query& query)
	{
		auto [transforms] = query.getComponents<TransformsComponent>(entity, true);
		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::AudioSource));
		}

		sound->get().init(*mApplication.getExternalTools().audioEngine);

		SOMBRA_INFO_LOG << "Entity " << entity << " with SoundComponent " << sound << " added successfully";
	}


	void AudioSystem::onRemoveSound(Entity entity, SoundComponent* sound, EntityDatabase::Query&)
	{
		SOMBRA_INFO_LOG << "Entity " << entity << " with SoundComponent " << sound << " removed successfully";
	}


	void AudioSystem::onNewTransforms(Entity, TransformsComponent* transforms, EntityDatabase::Query&)
	{
		transforms->updated.reset(static_cast<int>(TransformsComponent::Update::AudioSource));
	}


	void AudioSystem::onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event)
	{
		SOMBRA_INFO_LOG << event;

		std::scoped_lock lock(mMutex);
		mListenerEntity = event.getValue();
	}

}
