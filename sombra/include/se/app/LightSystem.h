#ifndef LIGHT_SYSTEM_H
#define LIGHT_SYSTEM_H

#include <memory>
#include <unordered_map>
#include "../graphics/core/Program.h"
#include "../graphics/core/UniformVariable.h"
#include "../graphics/3D/Mesh.h"
#include "../graphics/Pass.h"
#include "../graphics/Technique.h"
#include "events/ResizeEvent.h"
#include "events/ContainerEvent.h"
#include "ECS.h"

namespace se::app {

	class Application;
	class LightComponent;


	/**
	 * Class LightSystem, it's the System used for updating the Entities'
	 * LightComponents
	 */
	class LightSystem : public ISystem
	{
	private:	// Nested types
		template <typename T>
		using UniformVVSPtr = std::shared_ptr<
			graphics::UniformVariableValue<T>
		>;

		struct EntityUniforms
		{
			UniformVVSPtr<glm::mat4> modelMatrices[2];
			UniformVVSPtr<unsigned int> type;
			UniformVVSPtr<glm::vec3> color;
			UniformVVSPtr<float> intensity;
			UniformVVSPtr<float> range;
			UniformVVSPtr<float> lightAngleScale;
			UniformVVSPtr<float> lightAngleOffset;
		};

	private:	// Attributes
		/** The Application that holds the Entities */
		Application& mApplication;

		/** The Programs used for rendering the LightComponents
		 * RenderableMeshes */
		Repository::ResourceRef<graphics::Program> mProgramsDeferred[2];

		/** The stencil and color light passes used for rendering the
		 * LightComponents RenderableMeshes */
		Repository::ResourceRef<graphics::Pass> mPassesDeferred[2];

		/** The Technique used for rendering the LightComponents
		 * RenderableMeshes */
		Repository::ResourceRef<graphics::Technique> mTechniqueDeferred;

		/** The Mesh used for rendering PointLights */
		Repository::ResourceRef<graphics::Mesh> mPointLight;

		/** The Mesh used for rendering SpotLights */
		Repository::ResourceRef<graphics::Mesh> mSpotLight;

		/** The Mesh used for rendering DirectionalLights */
		Repository::ResourceRef<graphics::Mesh> mDirectionalLight;

		/** All the uniforms to update for each Entity */
		std::unordered_map<Entity, EntityUniforms> mEntityUniforms;

		/** The camera Entity used for rendering */
		Entity mCameraEntity;

		/** The camera position uniform variable for the deferred color light
		 * pass */
		UniformVVSPtr<glm::vec3> mCameraPosition;

	public:		// Functions
		/** Creates a new LightSystem
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered */
		LightSystem(
			Application& application, std::size_t width, std::size_t height
		);

		/** Class destructor */
		~LightSystem();

		/** @copydoc ISystem::notify(const IEvent&) */
		virtual bool notify(const IEvent& event) override;

		/** @copydoc ISystem::onNewComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onNewComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override;

		/** @copydoc ISystem::onRemoveComponent(Entity, const EntityDatabase::ComponentMask&) */
		virtual void onRemoveComponent(
			Entity entity, const EntityDatabase::ComponentMask& mask
		) override;

		/** Updates the LightSources with the Entities */
		virtual void update() override;
	private:
		/** Function called when a LightComponent is added to an Entity
		 *
		 * @param	entity the Entity that holds the LightComponent
		 * @param	light a pointer to the new LightComponent */
		void onNewLight(Entity entity, LightComponent* light);

		/** Function called when a LightComponent is going to be removed from
		 * an Entity
		 *
		 * @param	entity the Entity that holds the LightComponent
		 * @param	light a pointer to the LightComponent that is going to be
		 *			removed */
		void onRemoveLight(Entity entity, LightComponent* light);

		/** Handles the given ContainerEvent by updating the Camera Entity with
		 * which the Scene will be rendered
		 *
		 * @param	event the ContainerEvent to handle */
		void onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event);

		/** Handles the given RendererResolutionEvent by updating the
		 * Passes uniforms
		 *
		 * @param	event the RendererResolutionEvent to handle */
		void onRendererResolutionEvent(const RendererResolutionEvent& event);
	};

}

#endif		// LIGHT_SYSTEM_H
