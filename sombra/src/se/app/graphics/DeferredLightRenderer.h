#ifndef DEFERRED_LIGHT_RENDERER_H
#define DEFERRED_LIGHT_RENDERER_H

#include "se/graphics/BindableRenderNode.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/UniformBuffer.h"
#include "se/graphics/3D/Mesh.h"
#include "se/app/Repository.h"

namespace se::app {

	/**
	 * Class DeferredLightRenderer, It's the renderer used for calculating the
	 * lighting with the given g-buffer framebuffers.
	 * It has a Framebuffer "target" input and output, and "irradiance",
	 * "prefilter", "brdf", "shadow", "position", "normal", "albedo",
	 * "material" and "emissive" Texture inputs
	 */
	class DeferredLightRenderer : public graphics::BindableRenderNode
	{
	public:		// Nested types
		struct ShaderLightSource
		{
			glm::vec3 position;
			unsigned int type;
			glm::vec3 direction;
			int shadowIndices;
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
			static constexpr int kShadows		= 3;
			static constexpr int kPosition		= 4;
			static constexpr int kNormal		= 5;
			static constexpr int kAlbedo		= 6;
			static constexpr int kMaterial		= 7;
			static constexpr int kEmissive		= 8;
		};

		/** The maximum number of lights in the program */
		static constexpr unsigned int kMaxLights = 32;

	private:	// Attributes
		/** The program used by the DeferredLightRenderer */
		Repository::ResourceRef<graphics::Program> mProgram;

		/** The plane used for rendering */
		Repository::ResourceRef<graphics::Mesh> mPlane;

		/** The uniform variable that the Camera location in world space */
		std::shared_ptr<graphics::UniformVariableValue<glm::vec3>>
			mViewPosition;

		/** The uniform variable that holds the number of active lights to
		 * render */
		std::shared_ptr<graphics::UniformVariableValue<unsigned int>>
			mNumLights;

		/** The UniformBuffer where the lights data will be stored */
		std::shared_ptr<graphics::UniformBuffer> mLightsBuffer;

	public:
		/** Creates a new DeferredLightRenderer
		 *
		 * @param	name the name of the RenderNode
		 * @param	repository the Repository that holds the programs */
		DeferredLightRenderer(const std::string& name, Repository& repository);

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

		/** Executes the current RenderNode */
		virtual void execute() override;
	};

}

#endif		// DEFERRED_LIGHT_RENDERER_H
