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
		mEntityDatabase.addSystem(this, EntityDatabase::ComponentMask().set<ParticleSystemComponent>());
	}


	ParticleSystemSystem::~ParticleSystemSystem()
	{
		mEntityDatabase.removeSystem(this);
	}


	void ParticleSystemSystem::onNewEntity(Entity entity)
	{
		auto [particleSystem] = mEntityDatabase.getComponents<ParticleSystemComponent>(entity);
		if (!particleSystem) {
			SOMBRA_WARN_LOG << "Entity " << entity << " couldn't be added as ParticleSystem";
			return;
		}

		mApplication.getExternalTools().graphicsEngine->addRenderable(&particleSystem->get());
		SOMBRA_INFO_LOG << "Entity " << entity << " with ParticleSystem " << particleSystem << " added successfully";
	}


	void ParticleSystemSystem::onRemoveEntity(Entity entity)
	{
		auto [particleSystem] = mEntityDatabase.getComponents<ParticleSystemComponent>(entity);
		if (!particleSystem) {
			SOMBRA_INFO_LOG << "ParticleSystem Entity " << entity << " couldn't removed";
			return;
		}

		mApplication.getExternalTools().graphicsEngine->removeRenderable(&particleSystem->get());
		SOMBRA_INFO_LOG << "ParticleSystem Entity " << entity << " removed successfully";
	}


	void ParticleSystemSystem::update()
	{
		SOMBRA_DEBUG_LOG << "Updating the ParticleSystems";

		mEntityDatabase.iterateComponents<ParticleSystemComponent, TransformsComponent>(
			[this](Entity, ParticleSystemComponent* particleSystem, TransformsComponent* transforms) {
				if (!transforms->updated[static_cast<int>(TransformsComponent::Update::ParticleSystem)]) {
					particleSystem->setInitialPosition(transforms->position);
					particleSystem->setInitialOrientation(transforms->orientation);
					transforms->updated.set(static_cast<int>(TransformsComponent::Update::ParticleSystem));
				}
			}
		);

		mEntityDatabase.iterateComponents<ParticleSystemComponent>([this](Entity, ParticleSystemComponent* particleSystem) {
			particleSystem->update(mDeltaTime);
		});

		SOMBRA_INFO_LOG << "Update end";
	}

}
