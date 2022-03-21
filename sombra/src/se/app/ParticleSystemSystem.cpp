#include "se/utils/Log.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/app/ParticleSystemSystem.h"
#include "se/app/Application.h"
#include "se/app/TransformsComponent.h"
#include "se/app/ParticleSystemComponent.h"

namespace se::app {

	ParticleSystemSystem::ParticleSystemSystem(Application& application) :
		ISystem(application.getEntityDatabase()), mApplication(application)
	{
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask()
			.set<ParticleSystemComponent>()
			.set<TransformsComponent>()
		);
	}


	ParticleSystemSystem::~ParticleSystemSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void ParticleSystemSystem::onNewComponent(
		Entity entity, const EntityDatabase::ComponentMask& mask, EntityDatabase::Query& query
	) {
		tryCallC(&ParticleSystemSystem::onNewParticleSys, entity, mask, query);
		tryCallC(&ParticleSystemSystem::onNewTransforms, entity, mask, query);
	}


	void ParticleSystemSystem::onRemoveComponent(
		Entity entity, const EntityDatabase::ComponentMask& mask, EntityDatabase::Query& query
	) {
		tryCallC(&ParticleSystemSystem::onRemoveParticleSys, entity, mask, query);
	}


	void ParticleSystemSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the ParticleSystems";

		mEntityDatabase.executeQuery([this](EntityDatabase::Query& query) {
			query.iterateEntityComponents<ParticleSystemComponent, TransformsComponent>(
				[this](Entity, ParticleSystemComponent* particleSystem, TransformsComponent* transforms) {
					if (!transforms->updated[static_cast<int>(TransformsComponent::Update::ParticleSystem)]) {
						particleSystem->setInitialPosition(transforms->position);
						particleSystem->setInitialOrientation(transforms->orientation);
						transforms->updated.set(static_cast<int>(TransformsComponent::Update::ParticleSystem));
					}
				},
				true
			);

			query.iterateComponents<ParticleSystemComponent>(
				[this](ParticleSystemComponent& particleSystem) {
					particleSystem.update(mDeltaTime);
				},
				true
			);
		});

		SOMBRA_DEBUG_LOG << "Update end";
	}

// Private functions
	void ParticleSystemSystem::onNewParticleSys(Entity entity, ParticleSystemComponent* particleSystem, EntityDatabase::Query& query)
	{
		auto& context = mApplication.getExternalTools().graphicsEngine->getContext();
		particleSystem->setup(&mApplication.getEventManager(), &context, entity);

		auto [transforms] = query.getComponents<TransformsComponent>(entity, true);
		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::ParticleSystem));
		}

		mApplication.getExternalTools().graphicsEngine->addRenderable(&particleSystem->get());
		SOMBRA_INFO_LOG << "Entity " << entity << " with ParticleSystem " << particleSystem << " added successfully";
	}


	void ParticleSystemSystem::onRemoveParticleSys(Entity entity, ParticleSystemComponent* particleSystem, EntityDatabase::Query&)
	{
		mApplication.getExternalTools().graphicsEngine->removeRenderable(&particleSystem->get());

		auto& context = mApplication.getExternalTools().graphicsEngine->getContext();
		particleSystem->setup(nullptr, &context, kNullEntity);

		SOMBRA_INFO_LOG << "Entity " << entity << " with ParticleSystem " << particleSystem << " removed successfully";
	}


	void ParticleSystemSystem::onNewTransforms(Entity, TransformsComponent* transforms, EntityDatabase::Query&)
	{
		transforms->updated.reset(static_cast<int>(TransformsComponent::Update::ParticleSystem));
	}

}
