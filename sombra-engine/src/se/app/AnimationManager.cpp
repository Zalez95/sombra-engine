#include "se/utils/Log.h"
#include "se/app/AnimationManager.h"
#include "se/app/Entity.h"

namespace se::app {

	/*void AnimationManager::addEntity(Entity* entity, AnimationUPtr animation)
	{
		if (!entity || !animation) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		mAnimationSystem.addAnimation(animation.get());
		mAnimationEntities.emplace(entity, std::move(animation));
		SOMBRA_INFO_LOG << "Entity " << entity << " added successfully";
	}*/


	void AnimationManager::removeEntity(Entity* /*entity*/)
	{
		/*auto itAnimation = mAnimationEntities.find(entity);
		if (itAnimation != mAnimationEntities.end()) {
			mAnimationSystem.removeAnimation(itAnimation->second.get());
			mAnimationEntities.erase(itAnimation);
			SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
		}
		else {
			SOMBRA_WARN_LOG << "Entity " << entity << " wasn't removed";
		}*/
	}


	void AnimationManager::update(float delta)
	{
		SOMBRA_INFO_LOG << "Start (" << delta << ")";

		mAnimationSystem.update(delta);

		SOMBRA_INFO_LOG << "End";
	}

}
