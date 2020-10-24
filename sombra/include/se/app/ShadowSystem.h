#ifndef SHADOW_SYSTEM_H
#define SHADOW_SYSTEM_H

#include "IVPSystem.h"
#include "../graphics/RenderNode.h"
#include "events/ContainerEvent.h"
#include "CameraComponent.h"
#include "LightComponent.h"
#include "IVPSystem.h"

namespace se::app {

	/**
	 * Class ShadowSystem, it's the System used for updating the Entities
	 * the view and projection matrices of the shaders used for rendering the
	 * shadows.
	 * @note	it will only update the uniforms of the Passes that use the
	 *			"shadowRenderer"
	 */
	class ShadowSystem : public IVPSystem
	{
	private:	// Attributes
		/** The configuration used for rendering the shadows */
		ShadowData mShadowData;

		/** The Entity that holds the current light that creates the shadows */
		Entity mShadowEntity;

		/** The CameraComponent used for rendering the shadows */
		CameraComponent mShadowCamera;

		/** A pointer to the renderer used for rendering the shadows */
		graphics::RenderNode* mShadowRenderer;

	public:		// Functions
		/** Creates a new ShadowSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System
		 * @param	shadowData the configuration used for rendering the
		 *			Shadows */
		ShadowSystem(Application& application, const ShadowData& shadowData);

		/** Class destructor */
		~ShadowSystem();

		/** Notifies the ShadowSystem of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const IEvent& event) override;

		/** @copydoc IVPSystem::onRemoveEntity(Entity) */
		virtual void onRemoveEntity(Entity entity) override;

		/** Updates the LightSources with the Entities */
		virtual void update() override;
	private:
		/** @copydoc IVPSystem::getViewMatrix() */
		virtual glm::mat4 getViewMatrix() const override;

		/** @copydoc IVPSystem::getProjectionMatrix() */
		virtual glm::mat4 getProjectionMatrix() const override;

		/** @copydoc IVPSystem::shouldAddUniforms(PassSPtr) */
		virtual bool shouldAddUniforms(PassSPtr pass) const override;

		/** Handles the given ContainerEvent by updating the LightComponent
		 * Entity with which the shadows will be rendered
		 *
		 * @param	event the ContainerEvent to handle */
		void onShadowEvent(const ContainerEvent<Topic::Shadow, Entity>& event);
	};

}

#endif		// SHADOW_SYSTEM_H
