#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

namespace se::collision {

	/**
	 * Struct GraphVertex, it represents a vertex inside of a Graph
	 */
	template <typename T>
	struct GraphVertex
	{
		/** The id of the current vertex */
		int id;

		/** The data of the node */
		T data;

		/** The ids of the neighbour vertices sorted from lowest to
		 * highest */
		std::vector<int> neighbours;

		/** Creates a new GraphVertex */
		GraphVertex() : id(-1) {};

		/** Creates a new GraphVertex */
		GraphVertex(int id, T data) : id(id), data(data) {};

		/** Compares the given vertex with the current one
		 *
		 * @param	other the other graph vertex to compare with
		 * @return	true if the given vertex is the same than current one, false
		 *			otherwise */
		bool operator==(const GraphVertex<T>& other) const
		{ return id == other.id; }

		/** Compares the given vertex with the current one
		 *
		 * @param	other the other graph vertex to compare with
		 * @return	true if the given vertex id is less than current one, false
		 *			otherwise */
		bool operator<(const GraphVertex<T>& other) const
		{ return id < other.id; }

		/** Compares the vertex id with the current one
		 *
		 * @param	otherId the other graph vertex id to compare with
		 * @return	true if the given vertex id is less than current one, false
		 *			otherwise */
		bool operator<(int otherId) const { return id < otherId; }
	};


	/**
	 * Struct Graph, it's used to store a generic Graph
	 */
	template <typename T>
	struct Graph
	{
		/** The vertices of the graph sorted by id from lowest to highest */
		std::vector<GraphVertex<T>> vertices;
	};


	/** Collapses the given Graph nodes into the first one by removing second
	 * one and adding its relationships to the first one
	 *
	 * @param	iVertex1 the index of the graph node where second one is going
	 *			to be collapsed
	 * @param	iVertex2 the index of the graph node to collapse
	 * @param	dualGraph the graph where the nodes are located in */
	template <typename T>
	void halfEdgeCollapse(int iVertex1, int iVertex2, Graph<T>& dualGraph);

}

#include "Graph.inl"

#endif		// GRAPH_H
