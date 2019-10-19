#include "se/utils/Log.h"
#include "se/app/AnimationManager.h"
#include "se/app/Entity.h"

namespace se::app {

	void AnimationManager::addEntity(Entity* entity, animation::AnimationNode* animationNode)
	{
		if (!entity || !animationNode) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		mEntityNodes.emplace(entity, animationNode);
		SOMBRA_INFO_LOG << "Entity " << entity << " with AnimationNode " << animationNode << " added successfully";
	}


	void AnimationManager::removeEntity(Entity* entity)
	{
		auto itEntity = mEntityNodes.find(entity);
		if (itEntity != mEntityNodes.end()) {
			mEntityNodes.erase(itEntity);
			SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
		}
		else {
			SOMBRA_WARN_LOG << "Entity " << entity << " wasn't removed";
		}
	}


	void AnimationManager::update(float delta)
	{
		SOMBRA_INFO_LOG << "Start (" << delta << ")";

		// TODO: update the AnimationNode with the changes made to the entities
		mAnimationSystem.update(delta);
		// TODO: update the entities with the changes made to the AnimationNode

		SOMBRA_INFO_LOG << "End";
	}

}
