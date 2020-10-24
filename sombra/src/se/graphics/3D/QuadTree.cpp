#include <algorithm>
#include "se/graphics/3D/QuadTree.h"

namespace se::graphics {

	QuadTree::Node::Node() :
		children{ nullptr, nullptr, nullptr, nullptr }, isLeaf(true),
		parent(nullptr), quarterIndex(0), xzSeparation(0.0f),
		lod(0), neighboursLods{ -1, -1, -1, -1 } {}


	QuadTree::QuadTree(float size, const std::vector<float>& lodDistances) :
		mSize(size), mLodDistances(lodDistances)
	{
		mRootNode = std::make_unique<Node>();
	}


	void QuadTree::updateHighestLodLocation(const glm::vec3& highestLodLocation)
	{
		updateNode(*mRootNode, glm::vec2(0.0f), highestLodLocation);
	}

// Private functions
	void QuadTree::updateNode(
		Node& node, const glm::vec2& parentLocation, const glm::vec3& highestLodLocation
	) const
	{
		glm::vec2 nodeLocation = parentLocation + node.xzSeparation;
		float distance = glm::distance(highestLodLocation, { nodeLocation.x, 0.0f, nodeLocation.y });

		// If the distance to the highestLodLocation is closer than the lod one
		// the node should be splitted
		if ((node.lod < static_cast<int>(mLodDistances.size()) - 1)
			&& (distance < mLodDistances[node.lod])
		) {
			// Split the node if it's a leaf
			if (node.isLeaf) {
				split(node);
			}

			// Update the children nodes
			for (auto& child : node.children) {
				updateNode(*child, nodeLocation, highestLodLocation);
			}
		}
		else {
			if (!node.isLeaf) {
				// Collapse the children nodes
				for (auto& child : node.children) {
					updateNode(*child, nodeLocation, highestLodLocation);
				}

				// Collapse the node if it isn't a leaf and the lod difference
				// of its children (leaves) with their neighbours allows it
				if (std::all_of(
					node.children.begin(), node.children.end(),
					[](const auto& child) {
						return std::all_of(
							child->neighboursLods.begin(), child->neighboursLods.end(),
							[&](int lod) { return lod - child->lod <= 0; }
						);
					}
				)) {
					collapse(node);
				}
			}
		}
	}


	void QuadTree::split(Node& node) const
	{
		// Add the children nodes
		float childSeparation = mSize / std::pow(2.0f, node.lod + 2);
		for (unsigned char i = 0; i < 2; ++i) {
			for (unsigned char j = 0; j < 2; ++j) {
				auto child = std::make_unique<Node>();
				child->parent = &node;
				child->quarterIndex = 2*i + j;
				child->xzSeparation = { (j? 1 : -1) * childSeparation, (i? 1 : -1) * childSeparation };
				child->lod = node.lod + 1;
				node.children[child->quarterIndex] = std::move(child);
			}
		}

		// Change the leaf flag value
		node.isLeaf = false;

		// Notifies the neighbour nodes of the update, updating their lods and
		// dividing them if necessary
		updateNeighbours(node);
	}


	void QuadTree::collapse(Node& node) const
	{
		// Change the leaf flag value
		node.isLeaf = true;

		// Remove the children nodes recursively
		node.children = {};

		// Notifies the neighbour nodes of the update, updating their lods
		updateNeighbours(node);
	}


	void QuadTree::updateNeighbours(Node& node) const
	{
		if (node.isLeaf) {
			for (int i = 0; i < static_cast<int>(Direction::NumDirections); ++i) {
				Direction d = static_cast<Direction>(i);

				std::vector<Node*> path;
				node.neighboursLods[static_cast<int>(d)] = -1;
				for (Node* neighbour : getNeighbours(node, d, true, path)) {
					// Synch the neighbour lods array
					path.clear();
					neighbour->neighboursLods[static_cast<int>(inverse(d))] = -1;
					for (Node* neighbourNeighbour : getNeighbours(*neighbour, inverse(d), true, path)) {
						if (neighbourNeighbour->lod > neighbour->neighboursLods[static_cast<int>(inverse(d))]) {
							neighbour->neighboursLods[static_cast<int>(inverse(d))] = neighbourNeighbour->lod;
						}
					}
					if (neighbour->lod > node.neighboursLods[static_cast<int>(d)]) {
						node.neighboursLods[static_cast<int>(d)] = neighbour->lod;
					}

					// Split one of the nodes if necessary
					int lodDifference = node.lod - neighbour->lod;
					if (lodDifference > 1) {
						split(*neighbour);
					}
					else if (lodDifference < -1) {
						split(node);
					}
				}
			}
		}
		else {
			// Update all the children neighbours
			for (auto& child : node.children) {
				updateNeighbours(*child);
			}
		}
	}


	std::vector<QuadTree::Node*> QuadTree::getNeighbours(
		Node& currentNode, Direction neighbourDirection,
		bool isAscending, std::vector<Node*>& ascendingPath
	) {
		std::vector<Node*> ret;

		if (isAscending) {
			if (currentNode.lod == 0) {
				// No neighbour to notify in that direction
				ret = {};
			}
			else {
				// Continue ascending until the node isn't in the specified
				// direction
				ascendingPath.push_back(&currentNode);
				bool continueAscending = isAtDirection(currentNode.quarterIndex, neighbourDirection);
				ret = getNeighbours(*currentNode.parent, neighbourDirection, continueAscending, ascendingPath);
			}
		}
		else {
			if (currentNode.isLeaf) {
				return { &currentNode };
			}
			else if (ascendingPath.empty()) {
				// Descent through all the children nodes
				for (auto& child : currentNode.children) {
					if (isAtDirection(child->quarterIndex, inverse(neighbourDirection))) {
						auto childNeighbours = getNeighbours(*child, neighbourDirection, false, ascendingPath);
						ret.insert(ret.end(), childNeighbours.begin(), childNeighbours.end());
					}
				}
			}
			else {
				// Descend following the path
				Node& pathNode = *ascendingPath.back();
				ascendingPath.pop_back();

				Direction childDirection = (pathNode.parent == &currentNode)? neighbourDirection : inverse(neighbourDirection);
				unsigned char childIndex = selectChildren(pathNode.quarterIndex, childDirection);
				ret = getNeighbours(*currentNode.children[childIndex], neighbourDirection, false, ascendingPath);
			}
		}

		return ret;
	}


	constexpr QuadTree::Direction QuadTree::inverse(const Direction& direction)
	{
		return (direction == Direction::Bottom)? Direction::Top
			 : (direction == Direction::Top)? Direction::Bottom
			 : (direction == Direction::Left)? Direction::Right
			 : Direction::Left;
	}


	constexpr bool QuadTree::isAtDirection(unsigned char quarterIndex, Direction direction)
	{
		return ((direction == Direction::Bottom) && (quarterIndex / 2 == 0))
			|| ((direction == Direction::Top) && (quarterIndex / 2 == 1))
			|| ((direction == Direction::Left) && (quarterIndex % 2 == 0))
			|| ((direction == Direction::Right) && (quarterIndex % 2 == 1));
	}


	constexpr unsigned char QuadTree::selectChildren(unsigned char quarterIndex, Direction direction)
	{
		return (direction == Direction::Bottom)? (quarterIndex % 2)
			 : (direction == Direction::Top)? 2 + (quarterIndex % 2)
			 : (direction == Direction::Left)? 2 * (quarterIndex / 2)
			 : 1 + 2 * (quarterIndex / 2);
	}

}
