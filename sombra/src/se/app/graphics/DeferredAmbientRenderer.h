#ifndef DEFERRED_AMBIENT_RENDERER_H
#define DEFERRED_AMBIENT_RENDERER_H

#include "se/graphics/BindableRenderNode.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/3D/Mesh.h"
#include "se/app/Repository.h"

namespace se::app {

	/**
	 * Class DeferredAmbientRenderer, It's the renderer used for calculating the
	 * ambient lighting with the given g-buffer framebuffers.
	 * It has a Framebuffer "target" input and output, and "irradiance",
	 * "prefilter", "brdf", "position", "normal", "albedo", "material" and
	 * "emissive" Texture inputs
	 */
	class DeferredAmbientRenderer : public graphics::BindableRenderNode
	{
	public:		// Nested types
		struct TexUnits
		{
			static constexpr int kIrradianceMap	= 0;
			static constexpr int kPrefilterMap	= 1;
			static constexpr int kBRDFMap		= 2;
			static constexpr int kPosition		= 3;
			static constexpr int kNormal		= 4;
			static constexpr int kAlbedo		= 5;
			static constexpr int kMaterial		= 6;
			static constexpr int kEmissive		= 7;
		};

	private:	// Attributes
		/** The program used by the DeferredAmbientRenderer */
		Repository::ResourceRef<graphics::Program> mProgram;

		/** The plane used for rendering */
		Repository::ResourceRef<graphics::Mesh> mPlane;

		/** The uniform variable that the Camera location in world space */
		std::shared_ptr<graphics::UniformVariableValue<glm::vec3>>
			mViewPosition;

	public:
		/** Creates a new DeferredAmbientRenderer
		 *
		 * @param	name the name of the RenderNode
		 * @param	repository the Repository that holds the programs */
		DeferredAmbientRenderer(
			const std::string& name, Repository& repository
		);

		/** Class destructor */
		virtual ~DeferredAmbientRenderer() = default;

		/** Sets the position of the camera
		 *
		 * @param	position the new Camera location in world space */
		void setViewPosition(const glm::vec3& position);

		/** Executes the current RenderNode */
		virtual void execute() override;
	};

}

#endif		// DEFERRED_AMBIENT_RENDERER_H
