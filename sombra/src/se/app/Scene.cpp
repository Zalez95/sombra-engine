#include "se/animation/AnimationEngine.h"
#include "se/animation/SkeletonAnimator.h"
#include "se/audio/DataSource.h"
#include "se/physics/forces/Force.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/3D/Mesh.h"
#include "se/app/graphics/RenderableShader.h"
#include "se/app/SkinComponent.h"
#include "se/app/LightComponent.h"
#include "se/app/ScriptComponent.h"
#include "se/app/AnimationComponent.h"
#include "se/app/ParticleSystemComponent.h"
#include "se/app/Scene.h"

using namespace se::graphics;
using namespace se::physics;
using namespace se::audio;
using namespace se::animation;

namespace se::app {

	Scene::Scene(const std::string& name, Application& application) : application(application), name(name)
	{
		repository.init<ProgramRef>();
		repository.init<TextureRef>([](const TextureRef& texture) {
			auto tBindable = TextureRef::from(texture.clone());
			return std::make_unique<TextureRef>(tBindable);
		});
		repository.init<MeshRef>([](const MeshRef& mesh) {
			auto tBindable = MeshRef::from(mesh.clone());
			return std::make_unique<MeshRef>(tBindable);
		});
		repository.init<DataSource>([](const DataSource& dSource) {
			return std::make_unique<DataSource>(dSource);
		});
		repository.init<Force>([](const Force& force) {
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
		repository.init<SkeletonAnimator>([](const SkeletonAnimator& animator) {
			return std::unique_ptr<SkeletonAnimator>(dynamic_cast<SkeletonAnimator*>(animator.clone().release()));
		});
		repository.init<Script>([](const Script& script) {
			return script.clone();
		});

		entities.reserve(application.getEntityDatabase().getMaxEntities());
	}


	Scene::~Scene()
	{
		for (auto entity : entities) {
			application.getEntityDatabase().executeQuery([&](EntityDatabase::Query& query) {
				query.removeEntity(entity);
			});
		}
	}


	void removeEntityHierarchy(Scene& /*scene*/, Entity entity)
	{
		if (entity == kNullEntity) {
			return;
		}

		/*auto [animComponent] = scene.application.getEntityDatabase().getComponents<AnimationComponent>(entity, true);
		if (animComponent) {
			animation::AnimationNode& nodeRef = *animComponent->getRootNode();

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
		}*/
	}

}
