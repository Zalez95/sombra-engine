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

namespace se::app {

	class Application;


	/**
	 * Class AppRenderer, It's a System used for creating the RenderGraph and
	 * rendering the Entities
	 */
	class AppRenderer : public ISystem
	{
	private:	// Nested types
		struct ShaderLightSource;
		using TextureSPtr = std::shared_ptr<graphics::Texture>;
		template <typename T>
		using UniformVariableSPtr
			= std::shared_ptr<graphics::UniformVariableValue<T>>;

	private:	// Attributes
		/** The maximum number of lights in the program */
		static constexpr unsigned int kMaxLights = 32;

		static constexpr int kPosition		= 0;
		static constexpr int kNormal		= 1;
		static constexpr int kAlbedo		= 2;
		static constexpr int kMaterial		= 3;
		static constexpr int kEmissive		= 4;
		static constexpr int kIrradianceMap	= 5;
		static constexpr int kPrefilterMap	= 6;
		static constexpr int kBRDFMap		= 7;
		static constexpr int kColor			= 0;
		static constexpr int kBright		= 1;

		/** The Application that holds the GraphicsEngine used for rendering
		 * the Entities */
		Application& mApplication;

		/** The UniformBuffer where the lights data will be stored */
		std::shared_ptr<graphics::UniformBuffer> mLightsBuffer;

		/** The uniform variable that holds the number of active lights to
		 * render */
		UniformVariableSPtr<unsigned int> mNumLights;

		/** The uniform variable that the Camera location in world space */
		UniformVariableSPtr<glm::vec3> mViewPosition;

		/** The irradiance Texture to render with */
		TextureSPtr mIrradianceMap;

		/** The prefilter Texture to render with */
		TextureSPtr mPrefilterMap;

		/** The plane RenderableMesh used for rendering */
		std::shared_ptr<graphics::RenderableMesh> mPlaneRenderable;

		/** The camera Entity used for rendering */
		Entity mCameraEntity;

		/** If the camera was updated or not */
		bool mCameraUpdated;

	public:		// Functions
		/** Creates a new AppRenderer
		 *
		 * @param	application a reference to the Application that holds the
		 *			current System
		 * @param	width the initial width of the FrameBuffer where the
		 *			Entities are going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Entities are going to be rendered */
		AppRenderer(
			Application& application, std::size_t width, std::size_t height
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

		/** Handles the given ResizeEvent by notifying the GraphicsEngine of
		 * the window resize
		 *
		 * @param	event the ResizeEvent to handle */
		void onResizeEvent(const ResizeEvent& event);
	};

}

#endif		// APP_RENDERER_H
