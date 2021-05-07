#include "se/animation/AnimationEngine.h"
#include "se/animation/SkeletonAnimator.h"
#include "se/audio/Buffer.h"
#include "se/physics/forces/Force.h"
#include "se/physics/constraints/Constraint.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/3D/Mesh.h"
#include "se/app/SkinComponent.h"
#include "se/app/LightComponent.h"
#include "se/app/ParticleSystemComponent.h"
#include "se/app/RenderableShader.h"
#include "se/app/Scene.h"

namespace se::app {

	Scene::Scene(const char* name, Application& application) : name(name), application(application)
	{
		repository.init<Key, Skin>();
		repository.init<Key, LightSource>();
		repository.init<Key, ParticleEmitter>();
		repository.init<Key, RenderableShader>();
		repository.init<Key, animation::SkeletonAnimator>();
		repository.init<Key, audio::Buffer>();
		repository.init<Key, physics::Force>();
		repository.init<Key, physics::Constraint>();
		repository.init<Key, graphics::Program>();
		repository.init<Key, graphics::Pass>();
		repository.init<Key, graphics::Texture>();
		repository.init<Key, graphics::Mesh>();
		repository.init<Key, ResourcePath<audio::Buffer>>();
		repository.init<Key, ResourcePath<graphics::Program>>();
		repository.init<Key, ResourcePath<graphics::Texture>>();

		entities.reserve(application.getEntityDatabase().getMaxEntities());
	}


	Scene::~Scene()
	{
		for (auto entity : entities) {
			application.getEntityDatabase().removeEntity(entity);
		}

		repository.iterate<Key, animation::SkeletonAnimator>([this](const Key&, std::shared_ptr<animation::SkeletonAnimator> v) {
			application.getExternalTools().animationEngine->removeAnimator(v.get());
		});
	}


	void removeEntityHierarchy(Scene& scene, Entity entity)
	{
		if (entity == kNullEntity) {
			return;
		}

		auto [node] = scene.application.getEntityDatabase().getComponents<animation::AnimationNode*>(entity);
		if (node) {
			animation::AnimationNode& nodeRef = **node;

			// Remove the descendant entities
			for (auto it = nodeRef.begin<utils::Traversal::DFSPostOrder>(); it != nodeRef.end<utils::Traversal::DFSPostOrder>(); ++it) {
				animation::AnimationNode* descendantNode = &(*it);
				Entity descendantEntity = scene.application.getEntityDatabase().getEntity<animation::AnimationNode*>(&descendantNode);
				if (descendantEntity != kNullEntity) {
					scene.application.getEntityDatabase().removeEntity(descendantEntity);
				}
			}

			// Remove the entity
			scene.application.getEntityDatabase().removeEntity(entity);

			// Remove the AnimationNodes
			auto parentNode = nodeRef.getParent();
			if (parentNode) {
				auto itNode = parentNode->find(nodeRef);
				if (itNode != parentNode->end()) {
					parentNode->erase(itNode);
				}
			}
		}
	}

}
