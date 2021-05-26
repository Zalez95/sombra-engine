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
			},
			true
		);

		mEntityDatabase.iterateComponents<ParticleSystemComponent>(
			[this](Entity, ParticleSystemComponent* particleSystem) {
				particleSystem->update(mDeltaTime);
			},
			true
		);

		SOMBRA_INFO_LOG << "Update end";
	}

// Private functions
	void ParticleSystemSystem::onNewParticleSys(Entity entity, ParticleSystemComponent* particleSystem)
	{
		auto [transforms] = mEntityDatabase.getComponents<TransformsComponent>(entity, true);
		if (transforms) {
			transforms->updated.reset(static_cast<int>(TransformsComponent::Update::ParticleSystem));
		}

		mApplication.getExternalTools().graphicsEngine->addRenderable(&particleSystem->get());
		SOMBRA_INFO_LOG << "Entity " << entity << " with ParticleSystem " << particleSystem << " added successfully";
	}


	void ParticleSystemSystem::onRemoveParticleSys(Entity entity, ParticleSystemComponent* particleSystem)
	{
		mApplication.getExternalTools().graphicsEngine->removeRenderable(&particleSystem->get());
		SOMBRA_INFO_LOG << "Entity " << entity << " with ParticleSystem " << particleSystem << " removed successfully";
	}

}
