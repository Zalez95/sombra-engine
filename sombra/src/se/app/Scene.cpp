#include "se/animation/AnimationEngine.h"
#include "se/animation/SkeletonAnimator.h"
#include "se/audio/Buffer.h"
#include "se/physics/forces/Force.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/3D/Mesh.h"
#include "se/app/graphics/RenderableShader.h"
#include "se/app/SkinComponent.h"
#include "se/app/LightComponent.h"
#include "se/app/ParticleSystemComponent.h"
#include "se/app/Scene.h"

namespace se::app {

	Scene::Scene(const std::string& name, Application& application) : application(application), name(name)
	{
		repository.init<graphics::Program>();
		repository.init<graphics::Texture>([](const graphics::Texture& texture) {
			return std::unique_ptr<graphics::Texture>(dynamic_cast<graphics::Texture*>(texture.clone().release()));
		});
		repository.init<graphics::Mesh>([](const graphics::Mesh& mesh) {
			return std::unique_ptr<graphics::Mesh>(dynamic_cast<graphics::Mesh*>(mesh.clone().release()));
		});
		repository.init<audio::Buffer>();
		repository.init<physics::Force>([](const physics::Force& force) {
			return force.clone();
		});
		repository.init<Skin>([](const Skin& skin) {
			return std::make_unique<Skin>(skin);
		});
		repository.init<LightSource>([](const LightSource& source) {
			return std::make_unique<LightSource>(source);
		});
		repository.init<ParticleEmitter>([](const ParticleEmitter& emitter) {
			return std::make_unique<ParticleEmitter>(emitter);
		});
		repository.init<RenderableShaderStep>([](const RenderableShaderStep& step) {
			return step.clone();
		});
		repository.init<RenderableShader>([](const RenderableShader& shader) {
			return shader.clone();
		});
		repository.init<animation::SkeletonAnimator>([](const animation::SkeletonAnimator& animator) {
			return std::unique_ptr<animation::SkeletonAnimator>(dynamic_cast<animation::SkeletonAnimator*>(animator.clone().release()));
		});

		entities.reserve(application.getEntityDatabase().getMaxEntities());
	}


	Scene::~Scene()
	{
		for (auto entity : entities) {
			application.getEntityDatabase().removeEntity(entity);
		}
	}


	void removeEntityHierarchy(Scene& scene, Entity entity)
	{
		if (entity == kNullEntity) {
			return;
		}

		auto [node] = scene.application.getEntityDatabase().getComponents<animation::AnimationNode*>(entity, true);
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
