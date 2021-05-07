#ifndef RENDERABLE_TERRAIN_H
#define RENDERABLE_TERRAIN_H

#include "../core/VertexBuffer.h"
#include "../core/VertexArray.h"
#include "QuadTree.h"
#include "Renderable3D.h"

namespace se::graphics {

	/**
	 * Class RenderableTerrain, its a Renderable3D that holds all the terrain
	 * data
	 */
	class RenderableTerrain : public Renderable3D
	{
	private:	// Attributes
		/** The matrix that transforms from local space to world space */
		glm::mat4 mModelMatrix = glm::mat4(1.0f);

		/** The terrain QuadTree */
		QuadTree mQuadTree;

		/** The maximum height of the vertices of the terrain */
		float mMaxHeight;

	public:		// Functions
		/** Creates a new RenderableTerrain
		 *
		 * @param	size the size of the terrain in the XZ plane
		 * @param	maxHeight the maximum height of the vertices of the terrain
		 * @param	lodDistances the minimum distance to the camera at each
		 *			level of detail */
		RenderableTerrain(
			float size, float maxHeight,
			const std::vector<float>& lodDistances
		);
		RenderableTerrain(const RenderableTerrain& other);
		RenderableTerrain(RenderableTerrain&& other) = default;

		/** Class destructor */
		~RenderableTerrain() = default;

		/** Assignment operator */
		RenderableTerrain& operator=(const RenderableTerrain& other);
		RenderableTerrain& operator=(RenderableTerrain&& other) = default;

		/** Sets the model matrix
		 *
		 * @param	modelMatrix local space to world space matrix
		 * @return	a reference to the current RenderableTerrain object */
		RenderableTerrain& setModelMatrix(const glm::mat4& modelMatrix);

		/** @return	the local space to world space matrix */
		const glm::mat4& getModelMatrix() const
		{ return mModelMatrix; };

		/** @return	the QuadTree of the RenderableTerrain */
		QuadTree& getQuadTree() { return mQuadTree; };

		/** @return	the size of the RenderableTerrain in the XZ plane */
		float getSize() const { return mQuadTree.getSize(); };

		/** @return	the minimum distance to the camera at each level of detail */
		const std::vector<float>& getLodDistances() const
		{ return mQuadTree.getLodDistances(); };

		/** Updates the terrain QuadTree according to the given highest lod
		 * location and the configured level of detail distances
		 *
		 * @param	location the point of the terrain with the highest lod
		 *			location in local space */
		void setHighestLodLocation(const glm::vec3& location);
	};

}

#endif		// RENDERABLE_TERRAIN_H
