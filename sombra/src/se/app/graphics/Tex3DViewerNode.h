#ifndef TEX3D_VIEWER_NODE_H
#define TEX3D_VIEWER_NODE_H

#include <glm/glm.hpp>
#include "se/graphics/BindableRenderNode.h"

namespace se::app {

	/**
	 * Class Tex3DViewerNode, it's a RenderNode used for visualizing the
	 * contents of a Cubic 3D Texture.
	 * The node has a framebuffer "target" and a "texture3D" as inputs and
	 * outputs. The 3D texture must have a power of 2 resolution.
	 * For viewing the Texture 3D a Pass with a uProjectionMatrix and a
	 * uViewMatrix mat4 uniforms must be created pointing to the current
	 * RenderNode.
	 */
	class Tex3DViewerNode : public graphics::BindableRenderNode
	{
	public:		// Attributes
		/** The texture unit where the 3D texture must be attached */
		static constexpr int kTextureUnit = 0;
	private:
		/** The resolution of the 3D texture in each axis */
		std::size_t mMaxSize;

		/** The minimum and maximum positions in the Scene */
		glm::vec3 mMinPosition, mMaxPosition;

		/** The number of instances of @see mCube to draw */
		std::size_t mNumInstances;

		/** The index of the model matrix uniform variable */
		std::size_t mModelMatrixIndex;

		/** The index of the mipmap level uniform variable */
		std::size_t mMipMapLevelIndex;

		/** The index of the cube Mesh used for rendering the grid */
		std::size_t mCubeIndex;

	public:		// Functions
		/** Creates a new Tex3DViewerNode
		 *
		 * @param	name the name of the new Tex3DViewerNode
		 * @param	context the Context used for creating the RenderNode
		 *			Bindables
		 * @param	maxSize the resolution of the 3D texture in each axis
		 * @note	the initial mipmap level to draw will be the maximum
		 *			available */
		Tex3DViewerNode(
			const std::string& name, graphics::Context& context,
			std::size_t maxSize
		);

		/** Sets the scene bounds
		 *
		 * @param	minPosition the minimum position in the Scene
		 * @param	maxPosition the maximum position in the Scene */
		void setSceneBounds(
			const glm::vec3& minPosition, const glm::vec3& maxPosition
		);

		/** Sets the mipmap level to draw
		 *
		 * @param	mipmapLevel the new mipmap level */
		void setMipMapLevel(float mipmapLevel);

		/** @copydoc graphics::RenderNode::execute(graphics::Context::Query&) */
		virtual void execute(graphics::Context::Query& q) override;
	};

}

#endif		// TEX3D_VIEWER_NODE_H
