#ifndef QUAD_TREE_H
#define QUAD_TREE_H

#include <array>
#include <memory>
#include <glm/glm.hpp>
#include "../../utils/PackedVector.h"

namespace se::graphics {

	/**
	 * Class QuadTree, it's a data structure used for dividing recursivelly
	 * a plane in four regions so we can have a higher geometry resolution in
	 * certain regions than others
	 */
	class QuadTree
	{
	public:		// Nested types
		/** Struct Node, holds the data of each node of the QuadTree */
		struct Node
		{
			/** The indices of the 4 child nodes of the current one in the
			 * following order: top-left, top-right, bottom-left,
			 * bottom-right */
			std::array<int, 4> children = { -1, -1, -1, -1 };

			/** If the Node is a leaf Node or not */
			bool isLeaf = true;

			/** The index of the parent Node of the current one */
			int parent = -1;

			/** The index of the current Node in the mChildren array of its
			 * parent Node */
			unsigned char quarterIndex = 0;

			/** The separation of the current Node to its parent in the XZ
			 * plane */
			glm::vec2 xzSeparation = glm::vec2(0.0f);

			/** The level of detal of the current Node */
			int lod = 0;

			/** The lods of the neighbour Nodes to the current one */
			std::array<int, 4> neighboursLods = { -1, -1, -1, -1 };
		};

		/** Each of the directions of a Node */
		enum class Direction : int
		{ Bottom = 0, Top, Left, Right, NumDirections };

	public:		// Attributes
		/** The index of the root node of the QuadTree */
		static constexpr int kIRootNode = 0;
	private:
		/** The size of the QuadTree in the XZ plane */
		float mSize;

		/** The minimum distance to the highestLodLocation at each level of
		 * detail, from higher distance (smallest lod) to lower distance
		 * (highest lod) */
		std::vector<float> mLodDistances;

		/** All the nodes of the QuadTree */
		utils::PackedVector<Node> mNodes;

	public:		// Functions
		/** Creates a new QuadTree
		 *
		 * @param	size the size of the QuadTree in the XZ plane
		 * @param	lodDistances the minimum distance to the highestLodLocation
		 *			at each level of detail. It must have at least LOD 0 */
		QuadTree(
			float size = 0.0f, const std::vector<float>& lodDistances = { 0.0f }
		);

		/** @return	the size of the QuadTree in the XZ plane */
		float getSize() const { return mSize; };

		/** Sets the size in the XZ plane of the QuadTree
		 *
		 * @param	size the new size of the QuadTree in the XZ plane */
		void setSize(float size);

		/** @return	the the minimum distance to the highestLodLocation
		 *			at each level of detail. It must have at least LOD 0 */
		const std::vector<float>& getLodDistances() const
		{ return mLodDistances; };

		/** Sets the LOD distances of the QuadTree
		 *
		 * @param	lodDistances the minimum distance to the highestLodLocation
		 *			at each level of detail */
		void setLodDistances(const std::vector<float>& lodDistances);

		/** @return	the root Node of the QuadTree */
		const utils::PackedVector<Node>& getNodes() const { return mNodes; };

		/** Updates the QuadTree nodes depending on the distance to the
		 * highestLodLocation and the level of details
		 *
		 * @param	highestLodLocation the location with the highest lod in
		 *			local space */
		void updateHighestLodLocation(const glm::vec3& highestLodLocation);
	private:
		/** Updates the given node, splitting or collapsing its nodes depending
		 * on the distance to the highestLodLocation and the level of details
		 *
		 * @param	iNode an index to the node to update
		 * @param	parentLocation the location in of the parent node relative
		 *			to the root one
		 * @param	highestLodLocation the location with the highest lod in
		 *			local space */
		void updateNode(
			int iNode, const glm::vec2& parentLocation,
			const glm::vec3& highestLodLocation
		);

		/** Splits the given node and updates its neighbours
		 *
		 * @param	iNode the index of the node to split
		 * @note	the node must be a leaf */
		void split(int iNode);

		/** Collapses the given node and updates its neighbours
		 *
		 * @param	iNode the index of the node to collapse
		 * @param	all the children nodes of the node must be leaves */
		void collapse(int iNode);

		/** Synchs the lods of the given node and its neighbours, and splits the
		 * nodes if their lod difference is larger than 1
		 *
		 * @param	node the index of the node to update with its neighbours */
		void updateNeighbours(int iNode);

		/** Calculates the neighbours nodes of the given one in the given
		 * direction
		 *
		 * @param	iCurrentNode the index of the node to gets its neighbours
		 * @param	neighbourDirection the direction in which the neighbour
		 *			nodes to retrieve are located
		 * @param	isAscending the current state of the algorithm. Initially
		 *			must be set to true
		 * @param	ascendingPath the visited nodes during the ascend state.
		 *			Initially must be an empty vector
		 * @return	a vector with the indices to all the neighbour nodes */
		std::vector<int> getNeighbours(
			int iCurrentNode, Direction neighbourDirection,
			bool isAscending, std::vector<int>& ascendingPath
		);

		/** Calculates the inverse of the given direction
		 *
		 * @param	direction the original direction
		 * @return	the inverted direction */
		static constexpr Direction inverse(const Direction& direction);

		/** Checks if the node at the given quarter index is located at the
		 * given direction
		 *
		 * @param	quarterIndex the index of the node to test
		 * @param	direction the direction to test
		 * @return	true if its located at the given direction, false
		 *			otherwise */
		static constexpr bool isAtDirection(
			unsigned char quarterIndex, Direction direction
		);

		/** Calculates the next children node based on the given direction and
		 * a neighbour quarter index
		 *
		 * @param	quarterIndex the index of the neighbour node
		 * @param	direction the direction in which the new node will be
		 *			located
		 * @return	the new child index */
		static constexpr unsigned char selectChildren(
			unsigned char quarterIndex, Direction direction
		);
	};

}

#endif		// QUAD_TREE_H
