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
		 * @param	repository the Repository that holds all the Programs and
		 *			a "plane" Mesh
		 * @param	width the width of the output Texture
		 * @param	height the height of the output Texture
		 * @param	horizontal if the blur is going to be applied in the
		 *			horizontal direction or the vertical one */
		GaussianBlurNode(
			const std::string& name, utils::Repository& repository,
			std::size_t width, std::size_t height, bool horizontal
		);

		/** Sets the dimensions of the output texture
		 *
		 * @param	width the new width of the output Texture
		 * @param	height the new height of the output Texture */
		void setTextureDimensions(std::size_t width, std::size_t height);

		/** Adds the Gaussian blur */
		virtual void execute() override;
	};

}

#endif		// GAUSSIAN_BLUR_NODE_H
