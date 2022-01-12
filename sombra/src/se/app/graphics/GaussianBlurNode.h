#ifndef GAUSSIAN_BLUR_NODE_H
#define GAUSSIAN_BLUR_NODE_H

#include "se/graphics/BindableRenderNode.h"

namespace se::app {

	/**
	 * Class GaussianBlurNode, its a BindableRenderNode used for applying
	 * Gaussian blur in one direction to a Texture. It has an "input" input
	 * where the Texture to blur can be attached and an "target" input and
	 * output FrameBuffer for the blurred Texture. It also has a "plane" input
	 * where a plane Mesh must be attached for rendering
	 */
	class GaussianBlurNode : public graphics::BindableRenderNode
	{
	public:		// Attributes
		/** The texture unit where the color texture must be attached */
		static constexpr int kColorTextureUnit = 0;
	private:
		/** The index of the plane Mesh used for rendering */
		std::size_t mPlaneIndex;

	public:		// Functions
		/** Creates a new GaussianBlurNode
		 *
		 * @param	name the name of the new GaussianBlurNode
		 * @param	context the Context used for creating the RenderNode
		 *			Bindables
		 * @param	horizontal if the blur is going to be applied in the
		 *			horizontal direction or the vertical one */
		GaussianBlurNode(
			const std::string& name, graphics::Context& context, bool horizontal
		);

		/** @copydoc graphics::RenderNode::execute(graphics::Context::Query&) */
		virtual void execute(graphics::Context::Query& q) override;
	};

}

#endif		// GAUSSIAN_BLUR_NODE_H
