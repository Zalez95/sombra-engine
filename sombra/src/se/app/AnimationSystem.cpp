#include "se/utils/Log.h"
#include "se/animation/AnimationEngine.h"
#include "se/app/AnimationSystem.h"
#include "se/app/Application.h"
#include "se/app/EntityDatabase.h"
#include "se/app/TransformsComponent.h"

namespace se::app {

	AnimationSystem::AnimationSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<animation::AnimationNode*>());
	}


	AnimationSystem::~AnimationSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void AnimationSystem::onNewEntity(Entity entity)
	{
		auto [transforms, node] = mEntityDatabase.getComponents<TransformsComponent, animation::AnimationNode*>(entity);
		if (!node) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		if (transforms && *node) {
			// The Entity initial data is overridden by the AnimationNode one
			transforms->position = (*node)->getData().worldTransforms.position;
			transforms->orientation = (*node)->getData().worldTransforms.orientation;
			transforms->scale = (*node)->getData().worldTransforms.scale;
			transforms->updated.set( static_cast<int>(TransformsComponent::Update::Animation) );
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with AnimationNode " << *node << " added successfully";
	}


	void AnimationSystem::onRemoveEntity(Entity entity)
	{
		auto [node] = mEntityDatabase.getComponents<animation::AnimationNode*>(entity);
		if (!node) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be removed";
			return;
		}

		SOMBRA_INFO_LOG << "Node " << *node << " of Entity " << entity << " removed successfully";
	}


	void AnimationSystem::update()
	{
		SOMBRA_INFO_LOG << "Start";

		// Update the AnimationNodes with the changes made to the Entities
		/* FIXME: first update changes arent't notified
		mEntityDatabase.iterateComponents<TransformsComponent, animation::AnimationNode*>(
			[this](Entity, TransformsComponent* transforms, animation::AnimationNode** node) {
				// Reset the Entity animation update
				transforms->updated.reset( static_cast<int>(TransformsComponent::Update::Animation) );

				if (transforms->updated.any()) {
					animation::NodeData& nodeData = (*node)->getData();
					animation::AnimationNode* parentNode = (*node)->getParent();
					if (parentNode) {
						animation::NodeData& parentData = parentNode->getData();
						nodeData.localTransforms.position = transforms->position - parentData.worldTransforms.position;
						nodeData.localTransforms.orientation = glm::inverse(parentData.worldTransforms.orientation) * transforms->orientation;
						nodeData.localTransforms.scale = (1.0f / parentData.worldTransforms.scale) * transforms->scale;
					}
					else {
						nodeData.localTransforms.position = transforms->position;
						nodeData.localTransforms.orientation = transforms->orientation;
						nodeData.localTransforms.scale = transforms->scale;
					}
					animation::updateWorldTransforms(**node);
				}
			}
		);*/

		mApplication.getExternalTools().animationEngine->update(mDeltaTime);

		// Update the Entities with the changes made to the AnimationNode
		mEntityDatabase.iterateComponents<TransformsComponent, animation::AnimationNode*>(
			[this](Entity, TransformsComponent* transforms, animation::AnimationNode** node) {
				if (*node) {
					animation::NodeData& nodeData = (*node)->getData();
					if (nodeData.animated) {
						transforms->position = nodeData.worldTransforms.position;
						transforms->orientation = nodeData.worldTransforms.orientation;
						transforms->scale = nodeData.worldTransforms.scale;
						transforms->updated.set( static_cast<int>(TransformsComponent::Update::Animation) );
					}
				}
			}
		);

		SOMBRA_INFO_LOG << "End";
	}

}
