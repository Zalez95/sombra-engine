#include <algorithm>
#include "fe/collision/Graph.h"

namespace fe { namespace collision {

	bool lessVertexId(const GraphVertex& vertex, int iVertex)
	{
		return vertex.id < iVertex;
	}


	void halfEdgeCollapse(int iVertex1, int iVertex2, Graph& dualGraph)
	{
		auto itVertex1 = std::lower_bound(dualGraph.vertices.begin(), dualGraph.vertices.end(), iVertex1, lessVertexId);
		auto itVertex2 = std::lower_bound(dualGraph.vertices.begin(), dualGraph.vertices.end(), iVertex2, lessVertexId);
		if ((itVertex1 == dualGraph.vertices.end()) || (itVertex2 == dualGraph.vertices.end())) { return; }

		// Get all the neighbour vertices of the vertex 2 but not of the
		// vertex 1
		std::vector<int> neighbourDifference;
		std::set_symmetric_difference(
			itVertex1->neighbours.begin(), itVertex1->neighbours.end(),
			itVertex2->neighbours.begin(), itVertex2->neighbours.end(),
			std::back_inserter(neighbourDifference)
		);

		// Add the vertex 2 neighbour vertices to the first vertex
		for (int iVertex : neighbourDifference) {
			itVertex1->neighbours.insert(
				std::lower_bound(itVertex1->neighbours.begin(), itVertex1->neighbours.end(), iVertex),
				iVertex
			);
		}

		// Remove all the vertex 2 neighbours
		for (int iVertex2Neighbour : itVertex2->neighbours) {
			auto itVertex2Neighbour = std::lower_bound(dualGraph.vertices.begin(), dualGraph.vertices.end(), iVertex2Neighbour, lessVertexId);
			if (itVertex2Neighbour != dualGraph.vertices.end()) {
				auto itVertex2NN = std::lower_bound(itVertex2Neighbour->neighbours.begin(), itVertex2Neighbour->neighbours.end(), iVertex2);
				if (itVertex2NN != itVertex2Neighbour->neighbours.end()) {
					itVertex2Neighbour->neighbours.erase(itVertex2NN);
				}
			}
		}
		itVertex2->neighbours.clear();

		// Remove the vertex 2
		dualGraph.vertices.release(iVertex2);
	}

}}
