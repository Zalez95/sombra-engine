#ifndef FXAA_NODE_H
#define FXAA_NODE_H

#include "se/graphics/BindableRenderNode.h"

namespace se::app {

	/**
	 * Class FXAANode, its a BindableRenderNode used for applying
	 * FXAA to a Texture. It has an "input" input where the Texture to apply
	 * the anti-aliasing can be attached and an "target" input and output
	 * FrameBuffer for the anti-aliased Texture. It also has a "plane" input
	 * where a plane Mesh must be attached for rendering
	 */
	class FXAANode : public graphics::BindableRenderNode
	{
	public:		// Attributes
		/** The texture unit where the color texture must be attached */
		static constexpr int kColorTextureUnit = 0;
	private:
		/** The index of the plane Mesh used for rendering */
		std::size_t mPlaneIndex;

	public:		// Functions
		/** Creates a new FXAANode
		 *
		 * @param	name the name of the new FXAANode
		 * @param	context the Context used for creating the RenderNode
		 *			Bindables */
		FXAANode(const std::string& name, graphics::Context& context);

		/** @copydoc graphics::RenderNode::execute(graphics::Context::Query&) */
		virtual void execute(graphics::Context::Query& q) override;
	};

}

#endif		// FXAA_NODE_H
