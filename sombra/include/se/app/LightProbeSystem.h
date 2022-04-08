#ifndef LIGHT_PROBE_SYSTEM_H
#define LIGHT_PROBE_SYSTEM_H

#include "ECS.h"
#include "LightProbeComponent.h"

namespace se::app {

	class Application;


	/**
	 * Class LightProbeSystem, It's a System used for updating the Entities'
	 * LightProbes */
	class LightProbeSystem : public ISystem
	{
	private:	// Attributes
		/** The Application that holds the GraphicsEngine used for rendering
		 * the Entities */
		Application& mApplication;

		/** The light Probe Entity used for rendering */
		Entity mLightProbeEntity;

		/** A pointer to the last irradiance Texture */
		TextureRef mLastIrradianceTexture;

		/** A pointer to the last prefilter Texture */
		TextureRef mLastPrefilterTexture;

		/** The mutex used for protecting @see mLightProbeEntity,
		 * @see mLastIrradianceTexture and @see mLastIrradianceTexture */
		std::mutex mMutex;

	public:		// Functions
		/** Creates a new LightProbeSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		LightProbeSystem(Application& application);

		/** Class destructor */
		~LightProbeSystem();

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override;

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&, EntityDatabase::Query&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask,
			EntityDatabase::Query& query
		) override;

		/** Updates the light sources with the Entities
		 * @copydoc ISystem::update(float, float) */
		virtual void update(float deltaTime, float timeSinceStart) override;
	private:
		/** Function called when a LightProbeComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the LightProbeComponent
		 * @param	lightProbe a pointer to the new LightProbeComponent
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onNewLightProbe(
			Entity entity, LightProbeComponent* lightProbe,
			EntityDatabase::Query& query
		);

		/** Function called when a LightProbeComponent is going to be removed
		 * from an Entity
		 *
		 * @param	entity the Entity that holds the LightProbeComponent
		 * @param	lightProbe a pointer to the LightProbeComponent that is
		 *			going to be removed
		 * @param	query the Query object used for interacting with the Entity
		 *			and its other Components */
		void onRemoveLightProbe(
			Entity entity, LightProbeComponent* lightProbe,
			EntityDatabase::Query& query
		);
	};

}

#endif		// LIGHT_PROBE_SYSTEM_H
