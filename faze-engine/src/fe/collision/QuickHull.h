#ifndef QUICK_HULL_H
#define QUICK_HULL_H

#include <vector>
#include <glm/glm.hpp>
#include "HalfEdgeMesh.h"

namespace fe { namespace collision {

	/**
	 * Class QuickHull, TODO:
	 */
	class QuickHull
	{
	private:	// Attributes
		/** The Calculated ConvexHull of the mesh */
		HalfEdgeMesh mConvexHull;

	public:		// Functions
		/** Calculates the ConvexHull from the given Mesh data
		 *
		 * @param	meshData the Data of the mesh to calculate the ConvexHull
		 *			from */
		QuickHull(const HalfEdgeMesh& meshData);

		/** Class destructor */
		~QuickHull() {};
	private:
		/** Calculates the initial simplex needed for calculating the QuickHull
		 * algorithm from the mesh vertices
		 *
		 * @param	meshData the Mesh with the vertices vertices with which we
		 *			will calculate the initial simplex */
		void createInitialHull(const HalfEdgeMesh& meshData);

		/** Calculates the vertices that are outside of the current Hull by the
		 * given face
		 *
		 * @param	iFace the index of the Face
		 * @param	meshData the Mesh data with the Vertices and Faces
		 * @return	the indexes of the vertices that in front of the given
		 *			face */
		std::vector<int> getVerticesOutside(
			int iFace,
			const HalfEdgeMesh& meshData
		) const;

		/** Calculates which of the given Vertices is the the furthest point in
		 * the Face normal direction
		 *
		 * @param	vertexIndices the index of the Vertices to check
		 * @param	iFace the index of the Face
		 * @param	meshData the Mesh data with the Vertices and Faces
		 * @return	the index of the furthest Vertex */
		int getFurthestVertex(
			const std::vector<int>& vertexIndices,
			int iFace, const HalfEdgeMesh& meshData
		) const;
	};

}}

#endif		// QUICK_HULL_H
