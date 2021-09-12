#ifndef FXAA_NODE_H
#define FXAA_NODE_H

#include "../../graphics/BindableRenderNode.h"
#include "../../graphics/core/FrameBuffer.h"
#include "../../graphics/core/Program.h"
#include "../../graphics/3D/Mesh.h"
#include "../Repository.h"

namespace se::app {

	/**
	 * Class FXAANode, its a BindableRenderNode used for applying
	 * FXAA to a Texture. It has an "input" input where the Texture to apply
	 * the anti-aliasing can be attached and an "target" input and output
	 * FrameBuffer for the anti-aliased Texture
	 */
	class FXAANode : public graphics::BindableRenderNode
	{
	public:		// Attributes
		/** The texture unit where the color texture must be attached */
		static constexpr int kColorTextureUnit = 0;
	private:
		/** The program used by the FXAANode */
		Repository::ResourceRef<graphics::Program> mProgram;

		/** The Mesh used for rendering to the FrameBuffers */
		Repository::ResourceRef<graphics::Mesh> mPlane;

	public:		// Functions
		/** Creates a new FXAANode
		 *
		 * @param	name the name of the new FXAANode
		 * @param	repository the Repository that holds all the Programs and
		 *			a "plane" Mesh */
		FXAANode(const std::string& name, Repository& repository);

		/** Executes the FXAA algorithm */
		virtual void execute() override;
	};

}

#endif		// FXAA_NODE_H
