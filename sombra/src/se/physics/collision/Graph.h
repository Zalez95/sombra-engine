#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

namespace se::physics {

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

		/** Compares the given GraphVertex
		 *
		 * @param	gv1 the first GraphVertex to compare
		 * @param	gv2 the second GraphVertex to compare
		 * @return	true if both GraphVertex are the same, false otherwise */
		template <typename U>
		friend bool operator==(
			const GraphVertex<U>& gv1,
			const GraphVertex<U>& gv2
		);

		/** Compares the given GraphVertex
		 *
		 * @param	gv1 the first GraphVertex to compare
		 * @param	gv2 the second GraphVertex to compare
		 * @return	true if the first GraphVertex has a smaller id than the
		 *			second one, false otherwise */
		template <typename U>
		friend bool operator<(
			const GraphVertex<U>& gv1,
			const GraphVertex<U>& gv2
		);

		/** Compares the given GraphVertex with the given index
		 *
		 * @param	gv the GraphVertex to compare
		 * @param	id the id to compare
		 * @return	true if the GraphVertex has a smaller id than the given one,
		 *			false otherwise */
		template <typename U>
		friend bool operator<(const GraphVertex<U>& gv, int id);
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

#include "Graph.hpp"

#endif		// GRAPH_H
