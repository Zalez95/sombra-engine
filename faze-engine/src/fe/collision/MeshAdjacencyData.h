#ifndef MESH_ADJACENCY_DATA_H
#define MESH_ADJACENCY_DATA_H

#include <map>
#include <vector>
#include <glm/glm.hpp>

namespace fe { namespace collision {

	/**
	 * Struct Vertex
	 */
	struct Vertex
	{
		/** The 3D coordinates of the vertex */
		glm::vec3 location;

		/** The index of one of the outgoing Edges */
		int edge;

		Vertex(const glm::vec3& location) : location(location), edge(-1) {};
		~Vertex() {};
	};


	/**
	 * Struct Edge
	 */
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


	/**
	 * Struct Face
	 */
	struct Face
	{
		/** The index of one of the Edges of the Triangle */
		int edge;

		Face() : edge(-1) {};
		~Face() {};
	};


	/**
	 * Class MeshAdjacencyData, it holds the data of a 3D Mesh in a Half-Edge
	 * data structure so we can store the adjacency of the Faces and Edges for
	 * faster computations
	 */
	class MeshAdjacencyData
	{
	private:		// Attributes
		/** The vertices of the Mesh */
		std::vector<Vertex> mVertices;

		/** The edges of the Mesh */
		std::vector<Edge> mEdges;

		/** The faces of the Mesh */
		std::vector<Face> mFaces;

		/** Maps two Vertex indices with the Edge that references them */
		std::map<std::pair<int, int>, int> mVertexEdgeMap;

	public:		// Functions
		/** @return	the total number of Vertices in the Mesh */
		int getNumVertices() const { return mVertices.size(); };

		/** @return	the total number of Edges in the Mesh */
		int getNumEdges() const { return mEdges.size(); };

		/** @return	the total number of Faces in the Mesh */
		int getNumFaces() const { return mFaces.size(); };

		/** Returns the requested Vertex
		 *
		 * @param	i the Vertex index
		 * @return	a reference to the Vertex */
		const Vertex& getVertex(int i) const { return mVertices[i]; };

		/** Adds the given point as a new Vertex in the MeshAdjacencyData
		 *
		 * @param	point the 3D coordintes of the new Vertex */
		void addVertex(const glm::vec3& point);

		/** Removes the given vertex and the Faces and Edges that
		 * references it
		 *
		 * @param	index the index of the Vertex to remove */
		// TODO: void removeVertex(int index);

		/** Returns the requested Edge
		 *
		 * @param	i the Edge index
		 * @return	a reference to the Edge */
		const Edge& getEdge(int i) const { return mEdges[i]; };

		/** Returns the requested Face
		 *
		 * @param	i the Face index
		 * @return	a reference to the Face */
		const Face& getFace(int i) const { return mFaces[i]; };

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


	/** Calculates the normal of the given Face
	 *
	 * @param	iFace the index of the Face
	 * @param	meshData the data of the Mesh where the Face is located in
	 * @return	the normal of the Face */
	glm::vec3 calculateFaceNormal(int iFace, const MeshAdjacencyData& meshData);


	/** Calculates the furthest point of the Mesh in the given direction with
	 * the Hill-Climbing algorithm
	 *
	 * @param	direction the direction in which we are going to search
	 * @param	meshData the Mesh data with the Vertices and Faces
	 * @return	the index of the furthest Mesh Vertex
	 * @note	the Mesh must be convex, otherwise the furthest point found
	 *			could be a local maximum */
	int getFurthestVertexInDirection(
		const glm::vec3& direction,
		const MeshAdjacencyData& meshData
	);


	/** Calculates the boundary of the convex hull as seen from the given
	 * eye point
	 *
	 * @param	eyePoint the 3D coordinates of the eye point
	 * @param	iFace the index of the initial Face from which we will start
	 *			searching
	 * @param	meshData the Mesh data with the Vertices and Faces
	 * @return	a pair with the list of Edge indices that represents the
	 *			boundary of the ConvexHull and the list of Face indices with the
	 *			visible faces
	 * @note	the initial Face must be visible from the eyePoint
	 *			perspective */
	std::pair<std::vector<int>, std::vector<int>> calculateHorizon(
		const glm::vec3& eyePoint,
		int iFace, const MeshAdjacencyData& meshData
	);

}}

#endif		// MESH_ADJACENCY_DATA_H
