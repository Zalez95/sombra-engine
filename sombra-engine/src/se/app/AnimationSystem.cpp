#include "se/utils/Log.h"
#include "se/app/AnimationSystem.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	AnimationSystem::AnimationSystem(EntityDatabase& entityDatabase, animation::AnimationEngine& animationEngine) :
		ISystem(entityDatabase), mAnimationEngine(animationEngine)
	{
		mRootNode = std::make_unique<animation::AnimationNode>( animation::NodeData("AnimationSystem") );
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<animation::AnimationNode>());
	}


	AnimationSystem::~AnimationSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void AnimationSystem::onNewEntity(Entity entity)
	{
		auto [transforms, node] = mEntityDatabase.getComponents<TransformsComponent, animation::AnimationNode>(entity);
		if (!node) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		if (transforms) {
			// The Entity initial data is overridden by the AnimationNode one
			transforms->position = node->getData().worldTransforms.position;
			transforms->orientation = node->getData().worldTransforms.orientation;
			transforms->scale = node->getData().worldTransforms.scale;
			transforms->updated.set( static_cast<int>(TransformsComponent::Update::Animation) );
		}

		//TODO:mRootNode->insert(mRootNode->cbegin(), std::move(node));
		SOMBRA_INFO_LOG << "Entity " << entity << " with AnimationNode " << node << " added successfully";
	}


	void AnimationSystem::onRemoveEntity(Entity entity)
	{
		auto [node] = mEntityDatabase.getComponents<animation::AnimationNode>(entity);
		if (!node) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be removed";
			return;
		}

		// Remove the nodes
		/*TODO: auto itNode = std::find(mRootNode->begin(), mRootNode->end(), *node);
		if (itNode != mRootNode->end()) {
			SOMBRA_INFO_LOG << "Node " << &(*itNode) << " of Entity " << entity << " removed successfully";
			mRootNode->erase(itNode);
		}*/

		SOMBRA_INFO_LOG << "Entity " << entity << " removed successfully";
	}


	void AnimationSystem::update()
	{
		SOMBRA_INFO_LOG << "Start";

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

		mAnimationEngine.update(mDeltaTime);

		// Update the Entities with the changes made to the AnimationNode
		mEntityDatabase.iterateComponents<TransformsComponent, animation::AnimationNode>(
			[this](Entity, TransformsComponent* transforms, animation::AnimationNode* node) {
				animation::NodeData& nodeData = node->getData();
				if (nodeData.animated) {
					transforms->position = nodeData.worldTransforms.position;
					transforms->orientation = nodeData.worldTransforms.orientation;
					transforms->scale = nodeData.worldTransforms.scale;
					transforms->updated.set( static_cast<int>(TransformsComponent::Update::Animation) );
				}
			}
		);

		SOMBRA_INFO_LOG << "End";
	}

}
