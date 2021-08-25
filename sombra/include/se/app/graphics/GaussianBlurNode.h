#ifndef GAUSSIAN_BLUR_NODE_H
#define GAUSSIAN_BLUR_NODE_H

#include <glm/glm.hpp>
#include "../../graphics/BindableRenderNode.h"
#include "../../graphics/core/FrameBuffer.h"
#include "../../graphics/core/Program.h"
#include "../../graphics/3D/Mesh.h"
#include "../Repository.h"

namespace se::app {

	/**
	 * Class GaussianBlurNode, its a BindableRenderNode used for applying
	 * Gaussian blur in one direction to a Texture. It has an "input" input
	 * where the Texture to blur can be attached and an "output" input and
	 * output Textures for the blurred Texture
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

		/** The bindable index of the texture where the blurred image will
		 * be stored */
		std::size_t mOutputTextureBindableIndex;

		/** A pointer to the FrameBuffer used for rendering the blurred image */
		graphics::FrameBuffer* mFrameBuffer;

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

		/** @copydoc graphics::BindableRenderNode::setBindable() */
		virtual void setBindable(
			std::size_t bindableIndex, const BindableSPtr& bindable
		) override;

		/** Adds the Gaussian blur */
		virtual void execute() override;
	};

}

#endif		// GAUSSIAN_BLUR_NODE_H
