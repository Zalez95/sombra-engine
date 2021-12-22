#ifndef LIGHT_PROBE_SYSTEM_H
#define LIGHT_PROBE_SYSTEM_H

#include <memory>
#include "../graphics/core/Texture.h"
#include "events/ContainerEvent.h"
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
		std::shared_ptr<graphics::Texture> mLastIrradianceTexture;

		/** A pointer to the last prefilter Texture */
		std::shared_ptr<graphics::Texture> mLastPrefilterTexture;

	public:		// Functions
		/** Creates a new LightProbeSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System */
		LightProbeSystem(Application& application);

		/** Class destructor */
		~LightProbeSystem();

		/** @copydoc ISystem::onNewComponent() */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&LightProbeSystem::onNewLightProbe, entity, mask); };

		/** @copydoc ISystem::onRemoveComponent() */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override
		{ tryCallC(&LightProbeSystem::onRemoveLightProbe, entity, mask); };

		/** Updates the light sources with the Entities
		 *
		 * @note	this function must be called from the thread with the
		 *			Graphics API context (probably thread 0) */
		virtual void update() override;
	private:
		/** Function called when a LightProbeComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the LightProbeComponent
		 * @param	lightProbe a pointer to the new LightProbeComponent */
		void onNewLightProbe(Entity entity, LightProbeComponent* lightProbe);

		/** Function called when a LightProbeComponent is going to be removed
		 * from an Entity
		 *
		 * @param	entity the Entity that holds the LightProbeComponent
		 * @param	lightProbe a pointer to the LightProbeComponent that is
		 *			going to be removed */
		void onRemoveLightProbe(Entity entity, LightProbeComponent* lightProbe);
	};

}

#endif		// LIGHT_PROBE_SYSTEM_H