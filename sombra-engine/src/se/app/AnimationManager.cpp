#include <algorithm>
#include "se/utils/Log.h"
#include "se/app/AnimationManager.h"
#include "se/app/Entity.h"

namespace se::app {

	AnimationManager::AnimationManager(animation::AnimationEngine& animationEngine) :
		mAnimationEngine(animationEngine)
	{
		mRootNode = std::make_unique<animation::AnimationNode>( animation::NodeData("AnimationManager") );
	}


	void AnimationManager::addEntity(Entity* entity, animation::AnimationNode* animationNode)
	{
		if (!entity || !animationNode) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		if (std::find(mRootNode->begin(), mRootNode->end(), *animationNode) == mRootNode->end()) {
			SOMBRA_WARN_LOG << "The AnimationNode " << animationNode << " must be a descendant of the root node";
			return;
		}

		// The Entity initial data is overridden by the AnimationNode one
		entity->position = animationNode->getData().worldTransforms.position;
		entity->orientation = animationNode->getData().worldTransforms.orientation;
		entity->scale = animationNode->getData().worldTransforms.scale;
		entity->updated.set( static_cast<int>(Entity::Update::Animation) );

		mNodeEntities.emplace(animationNode, NodeEntity{ entity, false });
		SOMBRA_INFO_LOG << "Entity " << entity << " with AnimationNode " << animationNode << " added successfully";
	}


	void AnimationManager::addEntity(Entity* entity, std::unique_ptr<animation::AnimationNode> animationNode)
	{
		if (!entity || !animationNode) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		// The Entity initial data is overridden by the AnimationNode one
		entity->position = animationNode->getData().worldTransforms.position;
		entity->orientation = animationNode->getData().worldTransforms.orientation;
		entity->scale = animationNode->getData().worldTransforms.scale;
		entity->updated.set( static_cast<int>(Entity::Update::Animation) );

		animation::AnimationNode* animationNodePtr = animationNode.get();
		mRootNode->insert(mRootNode->cbegin(), std::move(animationNode));
		mNodeEntities.emplace(animationNodePtr, NodeEntity{ entity, true });
		SOMBRA_INFO_LOG << "Entity " << entity << " with AnimationNode " << animationNodePtr << " added successfully";
	}


	void AnimationManager::removeEntity(Entity* entity)
	{
		auto itPair = std::find_if(
			mNodeEntities.begin(), mNodeEntities.end(),
			[&](const std::pair<animation::AnimationNode*, NodeEntity>& pair) { return pair.second.entity == entity; }
		);
		if (itPair != mNodeEntities.end()) {
			// Remove the pair from the map
			animation::AnimationNode* node = itPair->first;
			NodeEntity nodeEntity = itPair->second;

			mNodeEntities.erase(itPair);

			if (nodeEntity.nodeOwner) {
				// Remove the descendant pairs from the map
				for (auto itNode = node->begin(); itNode != node->end(); ++itNode) {
					auto itPair2 = mNodeEntities.find(&(*itNode));
					if (itPair2 != mNodeEntities.end()) {
						SOMBRA_INFO_LOG << "Entity " << itPair2->second.entity << " removed due to the removel of " << entity;
						mNodeEntities.erase(itPair2);
					}
				}

				// Remove the nodes
				auto itNode = std::find(mRootNode->begin(), mRootNode->end(), *node);
				if (itNode != mRootNode->end()) {
					SOMBRA_INFO_LOG << "Node " << &(*itNode) << " of Entity " << entity << " removed successfully";
					mRootNode->erase(itNode);
				}
			}

			SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
		}
		else {
			SOMBRA_WARN_LOG << "Entity " << entity << " wasn't removed";
		}
	}


	void AnimationManager::update(float delta)
	{
		SOMBRA_INFO_LOG << "Start (" << delta << ")";

		// Update the AnimationNodes with the changes made to the Entities
		/*for (auto itNode = mRootNode->begin(); itNode != mRootNode->end(); ++itNode) {
			auto itNodeEntity = mNodeEntities.find( &(*itNode) );
			if (itNodeEntity != mNodeEntities.end()) {
				Entity* entity = itNodeEntity->second.entity;

				// Reset the Entity animation update
				entity->updated.reset( static_cast<int>(Entity::Update::Animation) );

				if (entity->updated.any()) {
					animation::NodeData& nodeData = itNode->getData();
					animation::AnimationNode* parentNode = itNode->getParent();
					if (parentNode) {
						animation::NodeData& parentData = parentNode->getData();
						nodeData.localTransforms.position = entity->position - parentData.worldTransforms.position;
						nodeData.localTransforms.orientation = glm::inverse(parentData.worldTransforms.orientation) * entity->orientation;
						nodeData.localTransforms.scale = (1.0f / parentData.worldTransforms.scale) * entity->scale;
					}
					else {
						nodeData.localTransforms.position = entity->position;
						nodeData.localTransforms.orientation = entity->orientation;
						nodeData.localTransforms.scale = entity->scale;
					}
					animation::updateWorldTransforms(*itNode);
				}
			}
		}*/

		mAnimationEngine.update(delta);

		// Update the Entities with the changes made to the AnimationNode
		for (auto& pair : mNodeEntities) {
			animation::AnimationNode* node = pair.first;
			Entity* entity = pair.second.entity;

			animation::NodeData& nodeData = node->getData();
			//if (nodeData.animated) {
				entity->position = nodeData.worldTransforms.position;
				entity->orientation = nodeData.worldTransforms.orientation;
				entity->scale = nodeData.worldTransforms.scale;
				entity->updated.set( static_cast<int>(Entity::Update::Animation) );
			//}
		}

		SOMBRA_INFO_LOG << "End";
	}

}
