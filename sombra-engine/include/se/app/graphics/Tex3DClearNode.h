#ifndef TEX3D_CLEAR_NODE
#define TEX3D_CLEAR_NODE

#include <glm/glm.hpp>
#include "../../graphics/BindableRenderNode.h"
#include "../../graphics/3D/Mesh.h"
#include "../../utils/Repository.h"

namespace se::app {

	/**
	 * Class Tex3DClearNode, it's a RenderNode used for clearing the
	 * contents of a Cubic 3D Texture.
	 * The node has a "input" and an "output" for attaching the 3D Texture
	 */
	class Tex3DClearNode : public graphics::BindableRenderNode
	{
	public:		// Attributes
		/** The image unit where the 3D texture must be attached */
		static constexpr int kImageUnit = 0;
	private:
		/** The resolution of the 3D texture in each axis */
		std::size_t mMaxSize;

		/** The plane Mesh used for clearing the Texture */
		std::unique_ptr<graphics::Mesh> mPlane;

	public:		// Functions
		/** Creates a new Tex3DClearNode
		 *
		 * @param	name the name of the new Tex3DClearNode
		 * @param	repository the Repository that holds all the Programs
		 * @param	maxSize the resolution of the 3D texture in each axis */
		Tex3DClearNode(
			const std::string& name, utils::Repository& repository,
			std::size_t maxSize
		);

		/** Clears the Texture 3D */
		virtual void execute() override;
	};

}

#endif		// TEX3D_CLEAR_NODE
