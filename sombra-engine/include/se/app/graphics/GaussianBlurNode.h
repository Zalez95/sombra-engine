#ifndef GAUSSIAN_BLUR_NODE_H
#define GAUSSIAN_BLUR_NODE_H

#include <glm/glm.hpp>
#include "../../graphics/BindableRenderNode.h"
#include "../../graphics/3D/RenderableMesh.h"
#include "../../utils/Repository.h"

namespace se::app {

	/**
	 * Class GaussianBlurNode, its a BindableRenderNode used for applying
	 * Gaussian blur in one direction to a Texture. It has an "input" input
	 * where the Texture can be attached and an "output" output where the
	 * blurred Texture can be retrieved
	 */
	class GaussianBlurNode : public graphics::BindableRenderNode
	{
	public:		// Attributes
		/** The texture unit where the color texture must be attached */
		static constexpr int kColorTextureUnit = 0;
	private:
		/** The RenderableMesh used for rendering to the FrameBuffers */
		std::shared_ptr<graphics::RenderableMesh> mPlane;

	public:		// Functions
		/** Creates a new GaussianBlurNode
		 *
		 * @param	name the name of the new GaussianBlurNode
		 * @param	repository the Repository that holds all the Programs
		 * @param	plane the RenderableMesh used for rendering to the
		 *			FrameBuffers
		 * @param	width the width of the output Texture
		 * @param	height the height of the output Texture
		 * @param	horizontal if the blur is going to be applied in the
		 *			horizontal direction or the vertical one */
		GaussianBlurNode(
			const std::string& name, utils::Repository& repository,
			std::shared_ptr<graphics::RenderableMesh> plane,
			std::size_t width, std::size_t height, bool horizontal
		);

		/** Adds the Gaussian blur */
		virtual void execute() override;
	};

}

#endif		// GAUSSIAN_BLUR_NODE_H
