#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <algorithm>

namespace fe { namespace collision {

	/**
	 * Struct GraphVertex, it represents a vertex inside of a Graph
	 */
	template<typename T>
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

		/** Destructor */
		~GraphVertex() {};

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
	template<typename T>
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
	template<typename T>
	void halfEdgeCollapse(int iVertex1, int iVertex2, Graph<T>& dualGraph)
	{
		auto itVertex1 = std::lower_bound(dualGraph.vertices.begin(), dualGraph.vertices.end(), iVertex1);
		auto itVertex2 = std::lower_bound(dualGraph.vertices.begin(), dualGraph.vertices.end(), iVertex2);
		if ((itVertex1 == dualGraph.vertices.end()) || (itVertex2 == dualGraph.vertices.end())) { return; }

		// Get all the neighbour vertices of the vertex 2 but not of the
		// vertex 1
		std::vector<int> neighbourDifference;
		std::set_difference(
			itVertex2->neighbours.begin(), itVertex2->neighbours.end(),
			itVertex1->neighbours.begin(), itVertex1->neighbours.end(),
			std::back_inserter(neighbourDifference)
		);

		// Add new edges between the neighbourDifference vertices and the
		// vertex 1
		for (int iVertex : neighbourDifference) {
			if (iVertex != iVertex1) {
				auto itVertex = std::lower_bound(dualGraph.vertices.begin(), dualGraph.vertices.end(), iVertex);
				if (itVertex != dualGraph.vertices.end()) {
					itVertex1->neighbours.insert(
						std::lower_bound(itVertex1->neighbours.begin(), itVertex1->neighbours.end(), iVertex),
						iVertex
					);

					itVertex->neighbours.insert(
						std::lower_bound(itVertex->neighbours.begin(), itVertex->neighbours.end(), iVertex1),
						iVertex1
					);
				}
			}
		}

		// Remove the vertex 2 from its neighbours' neighbours vector
		for (int iVertex2Neighbour : itVertex2->neighbours) {
			auto itVertex2Neighbour = std::lower_bound(dualGraph.vertices.begin(), dualGraph.vertices.end(), iVertex2Neighbour);
			if (itVertex2Neighbour != dualGraph.vertices.end()) {
				auto itVertex2NN = std::lower_bound(itVertex2Neighbour->neighbours.begin(), itVertex2Neighbour->neighbours.end(), iVertex2);
				if (itVertex2NN != itVertex2Neighbour->neighbours.end()) {
					itVertex2Neighbour->neighbours.erase(itVertex2NN);
				}
			}
		}

		// Remove the vertex 2
		dualGraph.vertices.erase(itVertex2);
	}

}}

#endif		// GRAPH_H
