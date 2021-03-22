#include "se/utils/Log.h"
#include "se/animation/AnimationEngine.h"
#include "se/app/AnimationSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/AnimationComponent.h"
#include "se/app/SkinComponent.h"

namespace se::app {

	AnimationSystem::AnimationSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<AnimationComponent>());
	}


	AnimationSystem::~AnimationSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void AnimationSystem::onNewEntity(Entity entity)
	{
		auto [transforms, animation] = mEntityDatabase.getComponents<TransformsComponent, AnimationComponent>(entity);
		if (!animation) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added";
			return;
		}

		if (transforms) {
			transforms->updated.reset( static_cast<int>(TransformsComponent::Update::Animation) );
		}

		SOMBRA_INFO_LOG << "Entity " << entity << " with AnimationNode " << animation << " added successfully";
	}


	void AnimationSystem::onRemoveEntity(Entity entity)
	{
		auto [animation] = mEntityDatabase.getComponents<AnimationComponent>(entity);
		if (!animation) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be removed";
			return;
		}

		SOMBRA_INFO_LOG << "Animation " << animation << " of Entity " << entity << " removed successfully";
	}


	void AnimationSystem::update()
	{
		SOMBRA_INFO_LOG << "Start";

		// Update the AnimationNodes with the changes made to the Entities
		mEntityDatabase.iterateComponents<TransformsComponent, AnimationComponent>(
			[this](Entity, TransformsComponent* transforms, AnimationComponent* animation) {
				if (animation->getRootNode() && !transforms->updated[static_cast<int>(TransformsComponent::Update::Animation)]) {
					animation::NodeData& nodeData = animation->getRootNode()->getData();
					animation::AnimationNode* parentNode = animation->getRootNode()->getParent();
					if (parentNode) {
						// FIXME: order matters!!! -> if children nodes are updated first than
						// the parents, the final node transforms are wrong
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
					animation::updateWorldTransforms(*animation->getRootNode());

					transforms->updated.set(static_cast<int>(TransformsComponent::Update::Animation));
				}
			}
		);

		mApplication.getExternalTools().animationEngine->update(mDeltaTime);

		// Update the Entities with the changes made to the AnimationNodes
		mEntityDatabase.iterateComponents<TransformsComponent, AnimationComponent>(
			[this](Entity, TransformsComponent* transforms, AnimationComponent* animation) {
				if (animation->getRootNode() && animation->getRootNode()->getData().animated) {
					animation::NodeData& nodeData = animation->getRootNode()->getData();
					transforms->position = nodeData.worldTransforms.position;
					transforms->orientation = nodeData.worldTransforms.orientation;
					transforms->scale = nodeData.worldTransforms.scale;
					transforms->updated.reset().set(static_cast<int>(TransformsComponent::Update::Animation));
				}
			}
		);
		mEntityDatabase.iterateComponents<TransformsComponent, SkinComponent>(
			[this](Entity, TransformsComponent* transforms, SkinComponent* skin) {
				skin->processNodes([&](const animation::AnimationNode& node) {
					if (node.getData().animated) {
						transforms->updated.reset(static_cast<int>(TransformsComponent::Update::Skin));
					}
				});
			}
		);

		SOMBRA_INFO_LOG << "End";
	}

}
