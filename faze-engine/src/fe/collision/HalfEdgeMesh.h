#ifndef HALF_EDGE_MESH_H
#define HALF_EDGE_MESH_H

#include <map>
#include <set>
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

		Vertex() : location(0.0f), edge(-1) {};
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
	 * Class HEMeshVector, the vector used by the HalfEdgeMesh class for
	 * caching the released elements instead of erasing them.
	 */
	template<class T>
	class HEMeshVector
	{
	private:	// Attributes
		/** The raw data of the HEMeshVector */
		std::vector<T> mRawData;

		/** The indices to the freed elements of the Vector */
		std::set<std::size_t> mFreeIndices;

	public:		// Functions
		/** Creates a new HEMeshVector */
		HEMeshVector() {};

		/** Class destructor */
		~HEMeshVector() {};

		/** @return	the number of Elements in the HEMeshVector */
		std::size_t size() const { return mRawData.size(); };

		/** Returns	the Element i of the HEMeshVector
		 *
		 * @param	i the index of the Element
		 * @return	a reference to the Element */
		T& operator[](std::size_t i) { return mRawData[i]; };

		/** Returns	the Element i of the HEMeshVector
		 *
		 * @param	i the index of the Element
		 * @return	a const reference to the Element */
		const T& operator[](std::size_t i) const { return mRawData[i]; };

		/** Creates a new Element in the vector or reuses an already released
		 * one
		 *
		 * @return	the index of the element */
		std::size_t create();

		/** Marks the Element located at the given index as released for
		 * future use
		 *
		 * @param	i the index of the Element
		 * @note	by releasing the elements instead than erasing them we
		 *			don't have to iterate through the elements for fixing
		*			its indexes */
		void free(std::size_t i);
	};


	/**
	 * Class HalfEdgeMesh, it holds the data of a 3D Mesh in a Half-Edge
	 * data structure so we can store the adjacency of the Faces and Edges for
	 * faster computations
	 */
	class HalfEdgeMesh
	{
	private:		// Attributes
		/** The vertices of the Mesh */
		HEMeshVector<Vertex> mVertices;

		/** The edges of the Mesh */
		HEMeshVector<Edge> mEdges;

		/** The faces of the Mesh */
		HEMeshVector<Face> mFaces;

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

		/** Adds the given point as a new Vertex in the Mesh
		 *
		 * @param	point the 3D coordintes of the new Vertex
		 * @return	the index of the Vertex */
		int addVertex(const glm::vec3& point);

		/** Removes the given Vertex and the Faces and Edges that
		 * references it
		 *
		 * @param	i the index of the Vertex to remove */
		void removeVertex(int i);

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
		 * to the Mesh
		 *
		 * @param	vertexIndexes the indexes of the vertex of the new face
		 * @return	the index of the Face */
		int addFace(const std::vector<int>& vertexIndexes);

		/** Removes the given Face and the Edges that references it
		 *
		 * @param	i the index of the Face to remove */
		void removeFace(int i);

		/** Merges two faces into a single one by the given edge
		 *
		 * @param	iEdge the shared edge between the faces to merge */
		void mergeFace(int iEdge);
	};


	/** Calculates the normal of the given Face
	 *
	 * @param	iFace the index of the Face
	 * @param	meshData the data of the Mesh where the Face is located in
	 * @return	the normal of the Face */
	glm::vec3 calculateFaceNormal(int iFace, const HalfEdgeMesh& meshData);


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
		const HalfEdgeMesh& meshData
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
		int iFace, const HalfEdgeMesh& meshData
	);


// Template functions definition
	template<class T>
	std::size_t HEMeshVector<T>::create()
	{
		std::size_t index;
		if (mFreeIndices.empty()) {
			mRawData.emplace_back();
			index = mRawData.size() - 1;
		}
		else {
			auto it = mFreeIndices.begin();
			index = *it;
			mFreeIndices.erase(it);
		}

		return index;
	}


	template<class T>
	void HEMeshVector<T>::free(std::size_t i)
	{
		mFreeIndices.insert(i);
		mRawData[i] = T();
	}

}}

#endif		// HALF_EDGE_MESH_H
