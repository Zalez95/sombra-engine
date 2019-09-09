#include "se/utils/Log.h"
#include "se/app/AudioManager.h"
#include "se/app/Entity.h"

namespace se::app {

	void AudioManager::setListener(Entity* entity)
	{
		if (!entity) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be setted as Listener";
			return;
		}

		// The Listener initial data is overridden by the entity one
		mAudioEngine.setListenerPosition(entity->position);
		mAudioEngine.setListenerOrientation(
			glm::vec3(0.0f, 0.0f, -1.0f) * entity->orientation,
			glm::vec3(0.0f, 1.0f, 0.0)
		);
		mAudioEngine.setListenerVelocity(entity->velocity);

		// Add the Listener
		mListener = entity;
		SOMBRA_INFO_LOG << "Entity " << entity << " was setted as Listener";
	}


	void AudioManager::addSource(Entity* entity, SourceUPtr source)
	{
		if (!entity || !source) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		// The Source initial data is overridden by the entity one
		audio::Source* sPtr = source.get();
		sPtr->setPosition(entity->position);
		sPtr->setOrientation(
			glm::vec3(0.0f, 0.0f, -1.0f) * entity->orientation,
			glm::vec3(0.0f, 1.0f, 0.0)
		);
		sPtr->setVelocity(entity->velocity);

		// Add the source
		mSourceEntities.emplace(entity, std::move(source));
		SOMBRA_INFO_LOG << "Entity " << entity << " with Source " << sPtr << " added successfully";
	}


	void AudioManager::removeEntity(Entity* entity)
	{
		auto itSource = mSourceEntities.find(entity);
		if (itSource != mSourceEntities.end()) {
			mSourceEntities.erase(itSource);
			SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
		}
		else {
			SOMBRA_WARN_LOG << "Entity " << entity << " wasn't removed";
		}
	}


	void AudioManager::update()
	{
		SOMBRA_INFO_LOG << "Updating the AudioManager";

		SOMBRA_DEBUG_LOG << "Updating the Listener";
		if (mListener && mListener->updated.any()) {
			mAudioEngine.setListenerPosition(mListener->position);
			mAudioEngine.setListenerOrientation(
				glm::vec3(0.0f, 0.0f, -1.0f) * mListener->orientation,
				glm::vec3(0.0f, 1.0f, 0.0)
			);
			mAudioEngine.setListenerVelocity(mListener->velocity);
		}

		SOMBRA_DEBUG_LOG << "Updating the Sources";
		for (auto& pair : mSourceEntities) {
			Entity* entity = pair.first;
			audio::Source* source = pair.second.get();

			if (entity->updated.any()) {
				source->setPosition(entity->position);
				source->setOrientation(
					glm::vec3(0.0f, 0.0f, -1.0f) * entity->orientation,
					glm::vec3(0.0f, 1.0f, 0.0)
				);
				source->setVelocity(entity->velocity);
			}
		}

		SOMBRA_INFO_LOG << "AudioManager updated";
	}

}
