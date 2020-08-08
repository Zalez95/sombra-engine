#ifndef APP_RENDERER_H
#define APP_RENDERER_H

#include <glm/glm.hpp>
#include "../graphics/Pass.h"
#include "../graphics/GraphicsEngine.h"
#include "../graphics/core/Texture.h"
#include "../graphics/core/UniformBuffer.h"
#include "../graphics/core/UniformVariable.h"
#include "../graphics/3D/RenderableMesh.h"
#include "ISystem.h"
#include "Application.h"
#include "CameraSystem.h"

namespace se::app {

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

		/** The GraphicsEngine used for rendering the Scene */
		graphics::GraphicsEngine& mGraphicsEngine;

		/** The CameraSystem that holds the Passes data */
		CameraSystem& mCameraSystem;

		/** The lighting pass used for rendering */
		std::shared_ptr<graphics::Pass> mLightingPass;

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

		/** The BRDF Texture to render with */
		TextureSPtr mBrdfMap;

		/** The plane RenderableMesh used for rendering */
		std::shared_ptr<graphics::RenderableMesh> mPlaneRenderable;

	public:		// Functions
		/** Creates a new AppRenderer
		 *
		 * @param	application the Application that holds all the data
		 * @param	graphicsEngine the GraphicsEngine used for rendering the
		 *			Scene
		 * @param	cameraSystem the CameraSystem that holds the Passes data
		 * @param	width the initial width of the FrameBuffer where the
		 *			Scene is going to be rendered
		 * @param	height the initial height of the FrameBuffer where the
		 *			Scene is going to be rendered */
		AppRenderer(
			Application& application, graphics::GraphicsEngine& graphicsEngine,
			CameraSystem& cameraSystem, std::size_t width, std::size_t height
		);

		/** Sets the irradiance texture of the AppRenderer
		 *
		 * @param	texture	the new irradiance texture */
		void setIrradianceMap(TextureSPtr texture);

		/** Sets the prefiltered environment map texture of the AppRenderer
		 *
		 * @param	texture	the new prefilter texture */
		void setPrefilterMap(TextureSPtr texture);

		/** Sets the convoluted BRDF texture of the AppRenderer
		 *
		 * @param	texture	the new BRDF texture */
		void setBRDFMap(TextureSPtr texture);

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
	};

}

#endif		// APP_RENDERER_H
