#ifndef DEFERRED_LIGHT_RENDERER_H
#define DEFERRED_LIGHT_RENDERER_H

#include "../../utils/Repository.h"
#include "../../graphics/BindableRenderNode.h"
#include "../../graphics/core/UniformVariable.h"
#include "../../graphics/core/UniformBuffer.h"
#include "../../graphics/3D/RenderableMesh.h"

namespace se::app {

	/**
	 * Class DeferredLightRenderer, It's the renderer used for calculating the
	 * lighting with the given g-buffer framebuffers.
	 */
	class DeferredLightRenderer : public graphics::BindableRenderNode
	{
	public:		// Nested types
		struct ShaderLightSource
		{
			glm::vec3 position;
			float padding[1];
			glm::vec3 direction;
			unsigned int type;
			glm::vec4 color;
			float intensity;
			float range;
			float lightAngleScale;
			float lightAngleOffset;
		};

		struct TexUnits
		{
			static constexpr int kIrradianceMap	= 0;
			static constexpr int kPrefilterMap	= 1;
			static constexpr int kBRDFMap		= 2;
			static constexpr int kShadowMap		= 3;
			static constexpr int kPosition		= 4;
			static constexpr int kNormal		= 5;
			static constexpr int kAlbedo		= 6;
			static constexpr int kMaterial		= 7;
			static constexpr int kEmissive		= 8;
		};

		/** The maximum number of lights in the program */
		static constexpr unsigned int kMaxLights = 32;

	private:	// Attributes
		/** The plane used for rendering */
		std::shared_ptr<graphics::RenderableMesh> mPlane;

		/** The uniform variable that the Camera location in world space */
		std::shared_ptr<graphics::UniformVariableValue<glm::vec3>>
			mViewPosition;

		/** The uniform variable that holds the number of active lights to
		 * render */
		std::shared_ptr<graphics::UniformVariableValue<unsigned int>>
			mNumLights;

		/** The UniformBuffer where the lights data will be stored */
		std::shared_ptr<graphics::UniformBuffer> mLightsBuffer;

		/** The uniform variable that holds the index of the LightSource used
		 * for rendering the Shadows */
		std::shared_ptr<graphics::UniformVariableValue<unsigned int>>
			mShadowLightIndex;

		/** The uniform variable with the view-projection matrix of the shadow
		 * mapping */
		std::shared_ptr<graphics::UniformVariableValue<glm::mat4>>
			mShadowViewProjectionMatrix;

	public:
		/** Creates a new DeferredLightRenderer
		 *
		 * @param	name the name of the RenderNode
		 * @param	repository the Repository that holds the programs
		 * @param	plane the Plane used for rendering */
		DeferredLightRenderer(
			const std::string& name, utils::Repository& repository,
			std::shared_ptr<graphics::RenderableMesh> plane
		);

		/** Class destructor */
		virtual ~DeferredLightRenderer() = default;

		/** Sets the position of the camera
		 *
		 * @param	position the new Camera location in world space */
		void setViewPosition(const glm::vec3& position);

		/** Sets the lights uniform variables and buffers
		 *
		 * @param	lightSources a pointer to the lights data
		 * @param	lightSourceCount the number of lights in ligthSources */
		void setLights(
			const ShaderLightSource* lightSources, unsigned int lightSourceCount
		);

		/** Sets the shadow light index uniform
		 *
		 * @param	shadowLightIndex the index of the light used for casting
		 *			the shadows in the light sources array */
		void setShadowLightIndex(unsigned int shadowLightIndex);

		/** Sets the shadow view-projection matrix uniform
		 *
		 * @param	shadowViewProjectionMatrix the view-projection matrix used
		 *			for rendering the shadows */
		void setShadowViewProjectionMatrix(
			const glm::mat4& shadowViewProjectionMatrix
		);

		/** Executes the current RenderNode */
		virtual void execute() override;
	};

}

#endif		// DEFERRED_LIGHT_RENDERER_H
