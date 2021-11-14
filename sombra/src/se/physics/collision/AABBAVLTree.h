#ifndef AABB_AVL_TREE_H
#define AABB_AVL_TREE_H

#include "se/utils/PackedVector.h"
#include "se/physics/collision/Ray.h"

namespace se::physics {

	/**
	 * Class AABBAVLTree, it's an AVL Tree that stores AABBs, it can be used
	 * for testing ray casts and overlaps between AABBs. The nodes can be added
	 * or removed dynamically, the Tree will automatically rebalance itself
	 * with each change
	 */
	template <typename T>
	class AABBAVLTree
	{
	private:	// Nested types
		/** Holds all the Data of an AABB Tree Node */
		struct TreeNode
		{
			std::size_t parent = 0;
			std::size_t leftChild = 0;
			std::size_t rightChild = 0;
			std::size_t height = 0;			///< Starting from the bottom
			bool isLeaf = true;
			AABB aabb = {};
			T userData;						///< Only for leaf Nodes
		};

	private:	// Attributes
		/** All the Nodes of the Tree */
		utils::PackedVector<TreeNode> mNodes;

		/** The index of the root node in @see mNodes */
		std::size_t mRootIndex;

	public:		// Functions
		/** Adds a node to the AABB Tree
		 *
		 * @param	aabb the AABB of the new Node
		 * @param	userData the user data of the Node
		 * @return	the node id */
		std::size_t addNode(const AABB& aabb, const T& userData);

		/** @return	the number of nodes in the AABBAVLTree */
		std::size_t getNumNodes() const { return mNodes.size(); };

		/** Returns the user data of the given node
		 *
		 * @param	nodeId the id of the node to check
		 * @return	the user data of the node */
		const T& getNodeUserData(std::size_t nodeId) const
		{ return mNodes[nodeId].userData; };

		/** Returns the AABB of the given node
		 *
		 * @param	nodeId the id of the node to check
		 * @return	the AABB of the node */
		const AABB& getNodeAABB(std::size_t nodeId) const
		{ return mNodes[nodeId].aabb; };

		/** @return	the AABB of the root node of the Tree, this AABB wraps all
		 *			the other AABBs */
		const AABB& getRootNodeAABB() const { return mNodes[mRootIndex].aabb; };

		/** Removes the node with the same id than the given one from the
		 * AABB Tree
		 *
		 * @param	nodeId the id of the node to remove */
		void removeNode(std::size_t nodeId);

		/** Calculates all the leaf nodes in the Tree that are currently
		 * overlaping
		 *
		 * @param	epsilon the epsilon value used for the comparisons
		 * @param	callback the function that will be called for every pair of
		 *			leaf nodes in the Tree that are overlaping
		 *			with both node ids as parameter */
		template <typename F>
		void calculateAllOverlaps(float epsilon, F&& callback) const;

		/** Calculates all the leaf nodes that overlaps with the given AABB
		 *
		 * @param	aabb the AABB to compare
		 * @param	epsilon the epsilon value used for the comparisons
		 * @param	callback the function that will be called for every leaf
		 *			node overlaping with the AABB with the node id as
		 *			parameter */
		template <typename F>
		void calculateOverlapsWith(
			const AABB& aabb, float epsilon, F&& callback
		) const;

		/** Calculates all the leaf nodes that intersects with the given ray
		 *
		 * @param	ray the ray to test
		 * @param	epsilon the epsilon value used for the comparisons
		 * @param	callback the function that will be called for every leaf
		 *			node intersecting with the ray with the node id as
		 *			parameter */
		template <typename F>
		void calculateIntersectionsWith(
			const Ray& ray, float epsilon, F&& callback
		) const;
	private:
		/** Calculate the best sibling node to the given node based on the area
		 *
		 * @param	nodeIndex the node index to calculate its best sibling node
		 * @see ErinCatto_DynamicBVH_GDC2019 */
		std::size_t calculateBestSibling(std::size_t nodeIndex);

		/** Updates the ancestor nodes of the given one, updating their AABBs
		 * and heights, adn rotating nodes when necessary
		 *
		 * @param	nodeIndex the node index to update its ancestor nodes */
		void updateAncestors(std::size_t nodeIndex);

		/** Updates the height of the given node and rotates its brother
		 * by one of its childs if the tree is unbalanced
		 *
		 * @param	nodeIndex the node to check */
		void rotateNodes(std::size_t nodeIndex);

		/** Swaps the locations of the given nodes in the tree
		 *
		 * @param	parent the index of the first node to swap, the parent one
		 * @param	child the index of the second node to swap, the child one
		 * @note	both nodes must be internal nodes, not leaf ones */
		void swapNodes(std::size_t nodeIndex1, std::size_t nodeIndex2);

		/** Calculates the balance of the given node
		 *
		 * @param	nodeIndex the index of the node to calculate its balance
		 * @return	the balance, >0 if the left branch is higher, <0 if it's
		 *			lower or 0 if they are the same */
		int calculateBalance(std::size_t nodeIndex);
	};

}

#include "AABBAVLTree.hpp"

#endif		// AABB_AVL_TREE_H
