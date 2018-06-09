#ifndef MESH_ADJACENCY_DATA_H
#define MESH_ADJACENCY_DATA_H

#include <map>
#include <vector>
#include <glm/glm.hpp>

namespace fe { namespace collision {

	/** TODO: */
	struct Vertex
	{
		/** The 3D coordinates of the vertex */
		glm::vec3 location;

		/** The index of one of the outgoing Edges */
		int edge;

		Vertex(const glm::vec3& location) : location(location), edge(-1) {};
		~Vertex() {};
	};


	/** TODO: */
	struct Edge
	{
		/** The index of the vertex which the Edge is pointing to */
		int vertex;

		/** The index of the face of the Edge */
		int face;

		/** The index of the previous Edge in the same Face */
		int previousEdge;

		/** The index of the next Edge in the same Face */
		int nextEdge;

		/** The index of an Edge pointing in the opposite direction */
		int oppositeEdge;

		Edge() :
			vertex(-1), face(-1),
			previousEdge(-1), nextEdge(-1), oppositeEdge(-1) {};
		~Edge() {};
	};


	/** TODO: CCW order */
	struct Face
	{
		/** The index of one of the Edges of the Triangle */
		int edge;

		Face() : edge(-1) {};
		~Face() {};
	};


	/** TODO: */
	class MeshAdjacencyData
	{
	public:		// Attributes
		/** The vertices of the Mesh */
		std::vector<Vertex> vertices;

		/** The edges of the Mesh */
		std::vector<Edge> edges;

		/** The faces of the Mesh */
		std::vector<Face> faces;
	private:
		/** Maps two Vertex indices with the Edge that references them */
		std::map<std::pair<int, int>, int> mVertexEdgeMap;

	public:		// Functions
		/** Adds the given point as a new Vertex in the MeshAdjacencyData
		 *
		 * @param	point the 3D coordintes of the new Vertex */
		void addVertex(const glm::vec3& point);

		/** Removes the given vertex and the Faces and Edges that
		 * references it
		 *
		 * @param	index the index of the Vertex to remove */
		// TODO: void removeVertex(int index);

		/** Creates a new Face from the given vertex indexes and adds it
		 * to the MeshAdjacencyData
		 *
		 * @param	vertexIndexes the indexes of the vertex of the new face */
		void addFace(std::vector<int> vertexIndexes);

		/** Removes the given Face and the Edges that references it
		 *
		 * @param	index the index of the Face to remove */
		// TODO: void removeFace(int index);
	};

}}

#endif		// MESH_ADJACENCY_DATA_H
