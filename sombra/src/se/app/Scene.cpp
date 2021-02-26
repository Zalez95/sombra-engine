#include "se/animation/CompositeAnimator.h"
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
#include "se/app/RenderableShader.h"
#include "se/app/Scene.h"
#include "se/app/EntityDatabase.h"

namespace se::app {

	Scene::Scene(const char* name, Application& application) : name(name), application(application)
	{
		repository.init<std::string, Skin>();
		repository.init<std::string, LightSource>();
		repository.init<std::string, RenderableShader>();
		repository.init<std::string, animation::CompositeAnimator>();
		repository.init<std::string, audio::Buffer>();
		repository.init<std::string, physics::Force>();
		repository.init<std::string, physics::Constraint>();
		repository.init<std::string, graphics::Program>();
		repository.init<std::string, graphics::Pass>();
		repository.init<std::string, graphics::Technique>();
		repository.init<std::string, graphics::Texture>();
		repository.init<std::string, graphics::Mesh>();

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
