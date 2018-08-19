#ifndef GRAPH_H
#define GRAPH_H

#include "ContiguousVector.h"

namespace fe { namespace collision {

	/**
	 * Struct GraphVertex, it represents a vertex inside of a Graph
	 */
	struct GraphVertex
	{
		/** The id of the current vertex */
		int id;

		/** The ids of the neighbour vertices sorted from lowest to
		 * highest */
		std::vector<int> neighbours;

		GraphVertex() : id(-1) {};
		~GraphVertex() {};
	};


	/**
	 * Struct Graph, it's used to store a generic Graph
	 */
	struct Graph
	{
		/** The vertices of the graph sorted by id from lowest to
		 * highest */
		ContiguousVector<GraphVertex> vertices;
	};


	/** Compares the given vertex with the given id graph vertex
	 *
	 * @param	vertex the graph vertex to compare
	 * @param	iVertex the id of the graph vertex to compare
	 * @return	true if the vertex id is less than the given vertex id, false
	 *			otherwise */
	bool lessVertexId(const GraphVertex& vertex, int iVertex);


	/** Collapses the given Graph nodes into the first one by removing second
	 * one and adding its relationships to the first one
	 *
	 * @param	iVertex1 the index of the graph node where second one is going
	 *			to be collapsed
	 * @param	iVertex2 the index of the graph node to collapse
	 * @param	dualGraph the graph where the nodes are located in */
	void halfEdgeCollapse(int iVertex1, int iVertex2, Graph& dualGraph);

}}

#endif		// GRAPH_H
