#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <algorithm>

namespace se::physics {

	template <typename T>
	bool operator==(const GraphVertex<T>& gv1, const GraphVertex<T>& gv2)
	{
		return gv1.id == gv2.id;
	}


	template <typename T>
	bool operator<(const GraphVertex<T>& gv1, const GraphVertex<T>& gv2)
	{
		return gv1.id < gv2.id;
	}


	template <typename T>
	bool operator<(const GraphVertex<T>& gv, int id)
	{
		return gv.id < id;
	}


	template <typename T>
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

		// Remove the vertex 2
		dualGraph.vertices.erase(itVertex2);
	}

}

#endif		// GRAPH_HPP
