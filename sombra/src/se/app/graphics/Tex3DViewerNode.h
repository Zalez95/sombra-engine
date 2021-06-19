#ifndef TEX3D_VIEWER_NODE_H
#define TEX3D_VIEWER_NODE_H

#include <glm/glm.hpp>
#include "se/graphics/BindableRenderNode.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/3D/Mesh.h"
#include "se/app/Repository.h"

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

		/** The program used by the Tex3DViewerNode */
		Repository::ResourceRef<graphics::Program> mProgram;

		/** The Cube Mesh used for rendering the grid */
		std::unique_ptr<graphics::Mesh> mCube;

		/** The number of instances of @see mCube to draw */
		std::size_t mNumInstances;

		/** The index of the model matrix uniform variable */
		std::size_t mModelMatrix;

		/** The index of the mipmap level uniform variable */
		std::size_t mMipMapLevel;

	public:		// Functions
		/** Creates a new Tex3DViewerNode
		 *
		 * @param	name the name of the new Tex3DViewerNode
		 * @param	repository the Repository that holds all the Programs
		 * @param	maxSize the resolution of the 3D texture in each axis
		 * @note	the initial mipmap level to draw will be the maximum
		 *			available */
		Tex3DViewerNode(
			const std::string& name, Repository& repository,
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

		/** Draws the Texture 3D grid */
		virtual void execute() override;
	};

}

#endif		// TEX3D_VIEWER_NODE_H
