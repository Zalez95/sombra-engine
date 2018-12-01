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

		mListener = entity;
		SOMBRA_INFO_LOG << "Entity " << entity << " was setted as Listener";
	}


	void AudioManager::addSource(Entity* entity, SourceUPtr source)
	{
		if (!entity || !source) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		mSourceEntities.emplace(entity, std::move(source));
		SOMBRA_INFO_LOG << "Entity " << entity << " added successfully";
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

		// Update the listener
		if (mListener) {
			glm::vec3 forwardVector = glm::vec3(0, 0,-1) * mListener->orientation;
			glm::vec3 upVector		= glm::vec3(0, 1, 0);

			mAudioEngine.setListenerPosition(mListener->position);
			mAudioEngine.setListenerOrientation(forwardVector, upVector);
			mAudioEngine.setListenerVelocity(mListener->velocity);
		}

		// Update the update the sources
		for (auto& se : mSourceEntities) {
			glm::vec3 forwardVector = glm::vec3(0, 0,-1) * se.first->orientation;
			glm::vec3 upVector		= glm::vec3(0, 1, 0);

			se.second->setPosition(se.first->position);
			se.second->setOrientation(forwardVector, upVector);
			se.second->setVelocity(se.first->velocity);
		}

		SOMBRA_INFO_LOG << "AudioManager updated";
	}

}
