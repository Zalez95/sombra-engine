#ifndef TEX3D_CLEAR_NODE
#define TEX3D_CLEAR_NODE

#include <glm/glm.hpp>
#include "se/graphics/BindableRenderNode.h"

namespace se::app {

	/**
	 * Class Tex3DClearNode, it's a RenderNode used for clearing the
	 * contents of a Cubic 3D Texture.
	 * The node has a "input" and an "output" for attaching the 3D Texture, it
	 * also has a "plane" input where a plane Mesh must be attached for
	 * rendering
	 */
	class Tex3DClearNode : public graphics::BindableRenderNode
	{
	public:		// Attributes
		/** The image unit where the 3D texture must be attached */
		static constexpr int kImageUnit = 0;
	private:
		/** The resolution of the 3D texture in each axis */
		std::size_t mMaxSize;

		/** The index of the plane Mesh used for clearing the Texture */
		std::size_t mPlaneIndex;

	public:		// Functions
		/** Creates a new Tex3DClearNode
		 *
		 * @param	name the name of the new Tex3DClearNode
		 * @param	context the Context used for creating the RenderNode
		 *			Bindables
		 * @param	maxSize the resolution of the 3D texture in each axis */
		Tex3DClearNode(
			const std::string& name, graphics::Context& context,
			std::size_t maxSize
		);

		/** @copydoc graphics::RenderNode::execute(graphics::Context::Query&) */
		virtual void execute(graphics::Context::Query& q) override;
	};

}

#endif		// TEX3D_CLEAR_NODE
