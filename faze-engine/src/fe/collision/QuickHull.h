#ifndef QUICK_HULL_H
#define QUICK_HULL_H

#include <vector>
#include <glm/glm.hpp>
#include "MeshAdjacencyData.h"

namespace fe { namespace collision {

	/**
	 * Class QuickHull, TODO:
	 */
	class QuickHull
	{
	private:	// Attributes
		/** The Calculated ConvexHull of the mesh */
		MeshAdjacencyData mConvexHull;

	public:		// Functions
		/** Calculates the ConvexHull from the given Mesh data
		 *
		 * @param	meshData the Data of the mesh to calculate the ConvexHull
		 *			from */
		QuickHull(const MeshAdjacencyData& meshData);

		/** Class destructor */
		~QuickHull() {};
	private:
		/** Calculates the initial simplex needed for calculating the QuickHull
		 * algorithm from the mesh vertices
		 *
		 * @param	meshData the Mesh with the vertices vertices with which we
		 *			will calculate the initial simplex */
		void createInitialHull(const MeshAdjacencyData& meshData);

		/** Calculates the vertices that are outside of the current Hull by the
		 * given face
		 *
		 * @param	iFace the index of the Face
		 * @param	meshData the Mesh data with the Vertices and Faces
		 * @return	the indexes of the vertices that in front of the given
		 *			face */
		std::vector<int> getVerticesOutside(
			int iFace,
			const MeshAdjacencyData& meshData
		) const;

		/** Calculates which of the given vertices is the the furthest point in
		 * the Face normal direction
		 *
		 * @param	iFace the index of the Face
		 * @param	vertexIndices the index of the vertices to check
		 * @param	meshData the Mesh data with the Vertices and Faces
		 * @return	the index of the furthest point */
		int getFurthestPoint(
			int iFace, const std::vector<int>& vertexIndices,
			const MeshAdjacencyData& meshData
		) const;
	};

}}

#endif		// QUICK_HULL_H
