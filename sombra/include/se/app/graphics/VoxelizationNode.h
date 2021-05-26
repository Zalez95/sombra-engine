#ifndef VOXELIZATION_NODE_H
#define VOXELIZATION_NODE_H

#include <glm/glm.hpp>
#include "../../graphics/3D/Renderer3D.h"
#include "../Repository.h"

namespace se::app {

	/**
	 * Class VoxelizationNode , its a Renderer3D that instead of rasterizing
	 * the 3D scene to a FrameBuffer, voxelizes it and stores the result in a
	 * 3D texture. The values to store in the 3D texture will be calculated from
	 * a Material structure.
	 * The node only has an input "texture3D" and an output "texture3D".
	 */
	class VoxelizationNode : public graphics::Renderer3D
	{
	public:		// Attributes
		/** The image unit where the 3D voxel texture must be attached */
		static constexpr int kVoxelImageUnit = 0;
	private:
		/** The maximum number of Voxels in the 3D texture */
		std::size_t mMaxVoxels;

		/** The minimum and maximum positions in the Scene */
		glm::vec3 mMinPosition, mMaxPosition;

		/** The indices of the projection matrices uniform variables */
		std::size_t mProjectionMatrices[3];

		/** The index of the 3D texture used for voxelizing the scene */
		std::size_t mVoxelImage;

	public:		// Functions
		/** Creates a new VoxelizationNode
		 *
		 * @param	name the name of the new VoxelizationNode
		 * @param	repository the Repository that holds all the Programs
		 * @param	maxVoxels the maximum number of voxels in each dimension */
		VoxelizationNode(
			const std::string& name, Repository& repository,
			std::size_t maxVoxels
		);

		/** Sets the scene bounds
		 *
		 * @param	minPosition the minimum position in the Scene
		 * @param	maxPosition the maximum position in the Scene */
		void setSceneBounds(
			const glm::vec3& minPosition, const glm::vec3& maxPosition
		);

		/** Renders all the submitted Renderables */
		virtual void render() override;
	};

}

#endif		// VOXELIZATION_NODE_H
