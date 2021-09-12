#ifndef GAUSSIAN_BLUR_NODE_H
#define GAUSSIAN_BLUR_NODE_H

#include "../../graphics/BindableRenderNode.h"
#include "../../graphics/core/FrameBuffer.h"
#include "../../graphics/core/Program.h"
#include "../../graphics/3D/Mesh.h"
#include "../Repository.h"

namespace se::app {

	/**
	 * Class GaussianBlurNode, its a BindableRenderNode used for applying
	 * Gaussian blur in one direction to a Texture. It has an "input" input
	 * where the Texture to blur can be attached and an "target" input and
	 * output FrameBuffer for the blurred Texture
	 */
	class GaussianBlurNode : public graphics::BindableRenderNode
	{
	public:		// Attributes
		/** The texture unit where the color texture must be attached */
		static constexpr int kColorTextureUnit = 0;
	private:
		/** The program used by the GaussianBlurNode */
		Repository::ResourceRef<graphics::Program> mProgram;

		/** The Mesh used for rendering to the FrameBuffers */
		Repository::ResourceRef<graphics::Mesh> mPlane;

	public:		// Functions
		/** Creates a new GaussianBlurNode
		 *
		 * @param	name the name of the new GaussianBlurNode
		 * @param	repository the Repository that holds all the Programs and
		 *			a "plane" Mesh
		 * @param	horizontal if the blur is going to be applied in the
		 *			horizontal direction or the vertical one */
		GaussianBlurNode(
			const std::string& name, Repository& repository, bool horizontal
		);

		/** Blurs the Texture */
		virtual void execute() override;
	};

}

#endif		// GAUSSIAN_BLUR_NODE_H
