#include <cassert>
#include <algorithm>
#include "se/graphics/3D/QuadTree.h"

namespace se::graphics {

	QuadTree::QuadTree(float size, const std::vector<float>& lodDistances) :
		mSize(size), mLodDistances(lodDistances)
	{
		assert(!lodDistances.empty() && "lodDistances must have at least LOD 0");

		mNodes.emplace();
	}


	void QuadTree::setSize(float size)
	{
		mSize = size;
		mNodes.clear();
		mNodes.emplace();
	}


	void QuadTree::setLodDistances(const std::vector<float>& lodDistances)
	{
		assert(!lodDistances.empty() && "lodDistances must have at least LOD 0");

		mLodDistances = lodDistances;
		mNodes.clear();
		mNodes.emplace();
	}


	void QuadTree::updateHighestLodLocation(const glm::vec3& highestLodLocation)
	{
		updateNode(kIRootNode, glm::vec2(0.0f), highestLodLocation);
	}

// Private functions
	void QuadTree::updateNode(
		int iNode, const glm::vec2& parentLocation, const glm::vec3& highestLodLocation
	) {
		glm::vec2 nodeLocation = parentLocation + mNodes[iNode].xzSeparation;
		float distance = glm::distance(highestLodLocation, { nodeLocation.x, 0.0f, nodeLocation.y });

		// If the distance to the highestLodLocation is closer than the lod one
		// the node should be splitted
		if ((mNodes[iNode].lod < static_cast<int>(mLodDistances.size()) - 1)
			&& (distance < mLodDistances[mNodes[iNode].lod])
		) {
			// Split the node if it's a leaf
			if (mNodes[iNode].isLeaf) {
				split(iNode);
			}

			// Update the children nodes
			for (std::size_t iChild = 0; iChild < mNodes[iNode].children.size(); ++iChild) {
				updateNode(mNodes[iNode].children[iChild], nodeLocation, highestLodLocation);
			}
		}
		else {
			if (!mNodes[iNode].isLeaf) {
				// Collapse the children nodes
				for (std::size_t iChild = 0; iChild < mNodes[iNode].children.size(); ++iChild) {
					updateNode(mNodes[iNode].children[iChild], nodeLocation, highestLodLocation);
				}

				// Collapse the node if it isn't a leaf and the lod difference
				// of its children (leaves) with their neighbours allows it
				if (std::all_of(
					mNodes[iNode].children.begin(), mNodes[iNode].children.end(),
					[this](int iChild) {
						return std::all_of(
							mNodes[iChild].neighboursLods.begin(), mNodes[iChild].neighboursLods.end(),
							[&](int lod) { return (lod - mNodes[iChild].lod) <= 0; }
						);
					}
				)) {
					collapse(iNode);
				}
			}
		}
	}


	void QuadTree::split(int iNode)
	{
		// Add the children nodes
		float childSeparation = mSize / static_cast<float>(std::pow(2.0f, mNodes[iNode].lod + 2));
		for (unsigned char i = 0; i < 2; ++i) {
			for (unsigned char j = 0; j < 2; ++j) {
				auto itChild = mNodes.emplace();
				itChild->parent = iNode;
				itChild->quarterIndex = 2*i + j;
				itChild->xzSeparation = { (j? 1 : -1) * childSeparation, (i? 1 : -1) * childSeparation };
				itChild->lod = mNodes[iNode].lod + 1;
				mNodes[iNode].children[itChild->quarterIndex] = itChild.getIndex();
			}
		}

		// Change the leaf flag value
		mNodes[iNode].isLeaf = false;

		// Notifies the neighbour nodes of the update, updating their lods and
		// dividing them if necessary
		updateNeighbours(iNode);
	}


	void QuadTree::collapse(int iNode)
	{
		// Change the leaf flag value
		mNodes[iNode].isLeaf = true;

		// Remove the children nodes recursively
		mNodes[iNode].children = {};

		// Notifies the neighbour nodes of the update, updating their lods
		updateNeighbours(iNode);
	}


	void QuadTree::updateNeighbours(int iNode)
	{
		if (mNodes[iNode].isLeaf) {
			for (int i = 0; i < static_cast<int>(Direction::NumDirections); ++i) {
				Direction d = static_cast<Direction>(i);

				std::vector<int> path;
				mNodes[iNode].neighboursLods[static_cast<int>(d)] = -1;
				for (int iNeighbour : getNeighbours(iNode, d, true, path)) {
					// Synch the neighbour lods array
					path.clear();
					mNodes[iNeighbour].neighboursLods[static_cast<int>(inverse(d))] = -1;
					for (int iNeighbourNeighbour : getNeighbours(iNeighbour, inverse(d), true, path)) {
						if (mNodes[iNeighbourNeighbour].lod > mNodes[iNeighbour].neighboursLods[static_cast<int>(inverse(d))]) {
							mNodes[iNeighbour].neighboursLods[static_cast<int>(inverse(d))] = mNodes[iNeighbourNeighbour].lod;
						}
					}
					if (mNodes[iNeighbour].lod > mNodes[iNode].neighboursLods[static_cast<int>(d)]) {
						mNodes[iNode].neighboursLods[static_cast<int>(d)] = mNodes[iNeighbour].lod;
					}

					// Split one of the nodes if necessary
					int lodDifference = mNodes[iNode].lod - mNodes[iNeighbour].lod;
					if (lodDifference > 1) {
						split(iNeighbour);
					}
					else if (lodDifference < -1) {
						split(iNode);
					}
				}
			}
		}
		else {
			// Update all the children neighbours
			for (std::size_t iChild = 0; iChild < mNodes[iNode].children.size(); ++iChild) {
				updateNeighbours(mNodes[iNode].children[iChild]);
			}
		}
	}


	std::vector<int> QuadTree::getNeighbours(
		int iCurrentNode, Direction neighbourDirection,
		bool isAscending, std::vector<int>& ascendingPath
	) {
		std::vector<int> ret;

		if (isAscending) {
			if (mNodes[iCurrentNode].lod == 0) {
				// No neighbour to notify in that direction
				ret = {};
			}
			else {
				// Continue ascending until the node isn't in the specified
				// direction
				ascendingPath.push_back(iCurrentNode);
				bool continueAscending = isAtDirection(mNodes[iCurrentNode].quarterIndex, neighbourDirection);
				ret = getNeighbours(mNodes[iCurrentNode].parent, neighbourDirection, continueAscending, ascendingPath);
			}
		}
		else {
			if (mNodes[iCurrentNode].isLeaf) {
				return { iCurrentNode };
			}
			else if (ascendingPath.empty()) {
				// Descent through all the children nodes
				for (std::size_t iiChild = 0; iiChild < mNodes[iCurrentNode].children.size(); ++iiChild) {
					int iChild = mNodes[iCurrentNode].children[iiChild];
					if (isAtDirection(mNodes[iChild].quarterIndex, inverse(neighbourDirection))) {
						auto childNeighbours = getNeighbours(iChild, neighbourDirection, false, ascendingPath);
						ret.insert(ret.end(), childNeighbours.begin(), childNeighbours.end());
					}
				}
			}
			else {
				// Descend following the path
				int pathINode = ascendingPath.back();
				ascendingPath.pop_back();

				Direction childDirection = (mNodes[pathINode].parent == iCurrentNode)? neighbourDirection : inverse(neighbourDirection);
				unsigned char childQIndex = selectChildren(mNodes[pathINode].quarterIndex, childDirection);
				ret = getNeighbours(mNodes[iCurrentNode].children[childQIndex], neighbourDirection, false, ascendingPath);
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
		return ((direction == Direction::Bottom) && ((quarterIndex >> 1) == 0))
			|| ((direction == Direction::Top) && ((quarterIndex >> 1) == 1))
			|| ((direction == Direction::Left) && ((quarterIndex & 1) == 0))
			|| ((direction == Direction::Right) && ((quarterIndex & 1) == 1));
	}


	constexpr unsigned char QuadTree::selectChildren(unsigned char quarterIndex, Direction direction)
	{
		return (direction == Direction::Bottom)? (quarterIndex & 1)
			 : (direction == Direction::Top)? 2 + (quarterIndex & 1)
			 : (direction == Direction::Left)? 2 * (quarterIndex >> 1)
			 : 1 + 2 * (quarterIndex >> 1);
	}

}
