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
	private:	// Nested types
		/** Struct Patch, holds the common graphics data of each patch of a
		 * Terrain */
		class Patch
		{
		private:	// Attributes
			/** The number of vertices of the patch */
			std::size_t mNumVertices;

			/** The VBO that holds the patch vertices position */
			VertexBuffer mVBOXZPositions;

			/** The VBO that holds the location of the patch instances in local
			 * space */
			VertexBuffer mVBOXZLocations;

			/** The VBO that holds the lods of the patch instances */
			VertexBuffer mVBOLods;

			/** The VAO of the patch */
			VertexArray mVAO;

			/** The number of instances to render */
			std::size_t mInstanceCount;

			/** The location of the patch instances in local space */
			std::vector<glm::vec2> mXZLocations;

			/** The lods of the patch instances */
			std::vector<int> mLods;

		public:		// Functions
			/** Creates a new Patch
			 *
			 * @param	vertices a pointer to the vertices of the patch
			 * @param	numVertices the number of vertices */
			Patch(const float* vertices, std::size_t numVertices);

			/** Adds an instance to draw
			 *
			 * @param	nodeLocation the location of the instance in local space
			 * @param	lod the lod of the instance */
			void submitInstance(const glm::vec2& nodeLocation, int lod);

			/** Draws all the instances of the patch */
			void drawInstances();
		};

	private:	// Attributes
		/** The terrain QuadTree */
		QuadTree mQuadTree;

		/** The patch to render with all the vertices */
		Patch mNormal;

		/** The patches to render with some side face missing used for
		 * morphing between different lods */
		Patch mBottom, mTop, mLeft, mRight;

		/** The patches to render with some corner faces missing used for
		 * morphing between different lods */
		Patch mBottomLeft, mBottomRight, mTopLeft, mTopRight;

	public:		// Functions
		/** Creates a new RenderableTerrain
		 *
		 * @param	size the size of the terrain in the XZ plane
		 * @param	lodDistances the minimum distance to the camera at each
		 *			level of detail */
		RenderableTerrain(float size, const std::vector<float>& lodDistances);

		/** Updates the terrain QuadTree according to the given highest lod
		 * location and the configured level of detail distances
		 *
		 * @param	location the point of the terrain with the highest lod
		 *			location in local space */
		void setHighestLodLocation(const glm::vec3& location);

		/** @copydoc Renderable3D::draw() */
		virtual void draw() override;
	private:
		/** Set the given leaf node or its descendants as instances of a patch
		 * to render
		 *
		 * @param	node the QuadTree Node to draw
		 * @param	parentLocation the location of the parent node relative
		 *			to the root node */
		void submitNode(
			const QuadTree::Node& node,
			const glm::vec2& parentLocation
		);
	};

}

#endif		// RENDERABLE_TERRAIN_H
