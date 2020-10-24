#ifndef APP_RENDERER_H
#define APP_RENDERER_H

#include <glm/glm.hpp>
#include "../graphics/Pass.h"
#include "../graphics/core/Texture.h"
#include "../graphics/core/UniformBuffer.h"
#include "../graphics/core/UniformVariable.h"
#include "../graphics/3D/RenderableMesh.h"
#include "ISystem.h"
#include "events/ContainerEvent.h"
#include "events/ResizeEvent.h"
#include "CameraComponent.h"
#include "LightComponent.h"

namespace se::app {

	class Application;


	/**
	 * Class AppRenderer, It's a System used for creating the RenderGraph and
	 * rendering the Entities
	 * @note	the Passes can use either the "forwardRenderer" or the
	 *			"gBufferRenderer" of the RenderGraph for submitting the
	 *			Renderable3Ds. The "gBufferRenderer" is used for rendering
	 *			geometry in a deferred pipeline, the passes that uses this it
	 *			must output position, normal, albedo, material and emissive
	 *			textures in that order, later this textures will be used for
	 *			rendering in a PBR pipeline. The "forwardRenderer" is reserved
	 *			for special cases that can't be rendered this way */
	class AppRenderer : public ISystem
	{
	private:	// Nested types
		struct ShaderLightSource;
		struct EnvironmentTexUnits;
		struct DeferredTexUnits;
		using TextureSPtr = std::shared_ptr<graphics::Texture>;
		template <typename T>
		using UniformVariableSPtr
			= std::shared_ptr<graphics::UniformVariableValue<T>>;
		class ShadowRenderer;
		class CombineNode;

	private:	// Attributes
		/** The maximum number of lights in the program */
		static constexpr unsigned int kMaxLights = 32;

		/** The Application that holds the GraphicsEngine used for rendering
		 * the Entities */
		Application& mApplication;

		/** The configuration used for rendering the shadows */
		ShadowData mShadowData;

		/** The UniformBuffer where the lights data will be stored */
		std::shared_ptr<graphics::UniformBuffer> mLightsBuffer;

		/** The uniform variable that holds the number of active lights to
		 * render */
		UniformVariableSPtr<unsigned int> mNumLights;

		/** The uniform variable that holds the index of the LightSource used
		 * for rendering the Shadows */
		UniformVariableSPtr<unsigned int> mShadowLightIndex;

		/** The uniform variable that the Camera location in world space */
		UniformVariableSPtr<glm::vec3> mViewPosition;

		/** The uniform variable with the view matrix of the shadow mapping */
		UniformVariableSPtr<glm::mat4> mShadowViewMatrix;

		/** The uniform variable with the projection matrix of the shadow
		 * mapping */
		UniformVariableSPtr<glm::mat4> mShadowProjectionMatrix;

		/** The bindable index of the irradiance Texture to render with */
		std::size_t mIrradianceTextureResource;

		/** The bindable index of the prefilter Texture to render with */
		std::size_t mPrefilterTextureResource;

		/** The plane RenderableMesh used for rendering */
		std::shared_ptr<graphics::RenderableMesh> mPlaneRenderable;

		/** The camera Entity used for rendering */
		Entity mCameraEntity;

		/** The light source Entity used for shadow mapping */
		Entity mShadowEntity;

		/** The CameraComponent used for rendering the shadows */
		CameraComponent mShadowCamera;

	public:		// Functions
		/** Creates a new AppRenderer
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System
		 * @param	shadowData the configuration used for rendering the shadows
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered */
		AppRenderer(
			Application& application, const ShadowData& shadowData,
			std::size_t width, std::size_t height
		);

		/** Class destructor */
		~AppRenderer();

		/** Notifies the AppRenderer of the given event
		 *
		 * @param	event the IEvent to notify */
		virtual void notify(const IEvent& event) override;

		/** Function that the EntityDatabase will call when an Entity is
		 * added
		 *
		 * @param	entity the new Entity */
		virtual void onNewEntity(Entity entity);

		/** Function that the EntityDatabase will call when an Entity is
		 * removed
		 *
		 * @param	entity the Entity to remove */
		virtual void onRemoveEntity(Entity entity);

		/** Updates the light sources with the Entities
		 *
		 * @note	this function must be called from the thread with the
		 *			Graphics API context (probably thread 0) */
		virtual void update() override;

		/** Renders the graphics data of the Entities
		 *
		 * @note	this function must be called from the thread with the
		 *			Graphics API context (probably thread 0) */
		void render();
	private:
		/** Handles the given ContainerEvent by updating the Camera Entity with
		 * which the Entities will be rendered
		 *
		 * @param	event the ContainerEvent to handle */
		void onCameraEvent(const ContainerEvent<Topic::Camera, Entity>& event);

		/** Handles the given ContainerEvent by updating the Shadow Entity with
		 * which the shadow will be rendered
		 *
		 * @param	event the ContainerEvent to handle */
		void onShadowEvent(const ContainerEvent<Topic::Shadow, Entity>& event);

		/** Handles the given ResizeEvent by notifying the GraphicsEngine of
		 * the window resize
		 *
		 * @param	event the ResizeEvent to handle */
		void onResizeEvent(const ResizeEvent& event);
	};

}

#endif		// APP_RENDERER_H
