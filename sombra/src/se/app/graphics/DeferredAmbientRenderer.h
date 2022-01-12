#ifndef DEFERRED_AMBIENT_RENDERER_H
#define DEFERRED_AMBIENT_RENDERER_H

#include <glm/glm.hpp>
#include "se/graphics/BindableRenderNode.h"

namespace se::app {

	/**
	 * Class DeferredAmbientRenderer, It's the renderer used for calculating
	 * the ambient lighting with the given g-buffer framebuffers.
	 * It has a Framebuffer "target" input and output, and "irradiance",
	 * "prefilter", "brdf", "position", "normal", "albedo", "material" and
	 * "emissive" Texture inputs. It also has a "plane" input where a plane
	 * Mesh must be attached for rendering
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
			static constexpr int kSSAO			= 8;
		};

	private:	// Attributes
		/** The index of the plane Mesh used for rendering */
		std::size_t mPlaneIndex;

		/** The index of the uniform variable that the Camera location in
		 * world space */
		std::size_t mViewPositionIndex;

	public:
		/** Creates a new DeferredAmbientRenderer
		 *
		 * @param	name the name of the RenderNode
		 * @param	context the Context used for creating the RenderNode
		 *			Bindables */
		DeferredAmbientRenderer(
			const std::string& name, graphics::Context& context
		);

		/** Class destructor */
		virtual ~DeferredAmbientRenderer() = default;

		/** Sets the position of the camera
		 *
		 * @param	position the new Camera location in world space */
		void setViewPosition(const glm::vec3& position);

		/** @copydoc graphics::RenderNode::execute(graphics::Context::Query&) */
		virtual void execute(graphics::Context::Query& q) override;
	};

}

#endif		// DEFERRED_AMBIENT_RENDERER_H
