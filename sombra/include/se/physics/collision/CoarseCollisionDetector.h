#ifndef COARSE_COLLISION_DETECTOR_H
#define COARSE_COLLISION_DETECTOR_H

#include <functional>
#include "../../utils/PackedVector.h"
#include "AABB.h"

namespace se::physics {

	class Collider;


	/**
	 * Class CoarseCollisionDetector, it's used to detect which colliders are
	 * intersecting by their AABBs using an AABB Tree
	 */
	class CoarseCollisionDetector
	{
	public:		// Nested types
		using ColliderPair = std::pair<Collider*, Collider*>;
		using IntersectionCallback = std::function<void(const ColliderPair&)>;
		using ColliderCallback = std::function<void(Collider*)>;
	private:
		/** Holds cached data of a Collider */
		struct ColliderData
		{
			Collider* collider;
			std::size_t nodeIndex;
		};

		/** Holds all the Data of a AABB Tree Node */
		struct TreeNode
		{
			std::size_t colliderIndex = 0;
			std::size_t parent = 0;
			std::size_t leftChild = 0;
			std::size_t rightChild = 0;
			std::size_t height = 0;	///<starting from the bottom
			bool isLeaf = true;
			AABB aabb = {};
		};

	public:	// Attributes
		/** The epsilon value used for testing for raycasts */
		float mEpsilon;

		/** The Colliders to check if they collide between each other */
		utils::PackedVector<ColliderData> mColliders;

		/** All the Nodes of the Tree */
		utils::PackedVector<TreeNode> mNodes;

		/** The index of the root node in @see mNodes */
		std::size_t mRootIndex;

	public:	// Functions
		/** Creates a new CoarseCollisionDetector
		 *
		 * @param	epsilon the Epsilon value used for the tests */
		CoarseCollisionDetector(float epsilon) :
			mEpsilon(epsilon), mRootIndex(0) {};

		/** Adds the given Collider to the Detector so it will check if the
		 * Collider intersecs with the other Colliders in it
		 *
		 * @param	collider a pointer to the Collider to add */
		void add(Collider* collider);

		/** Iterates through all the Colliders added to the Detector
		 * calling the given callback function
		 *
		 * @param	callback the function to call for each Collider */
		void processColliders(const ColliderCallback& callback) const;

		/** Removes the given Collider from the Detector so it won't longer
		 * check if the Collider intersecs with the other Colliders in it
		 *
		 * @param	collider a pointer to the Collider to remove */
		void remove(Collider* collider);

		/** Updates the Detector with the movement of the Colliders, this must
		 * be called at every clock tick */
		void update();

		/** Calculates all the Colliders that are currently intersecting
		 *
		 * @param	callback the function that must be called for
		 *			every pair of Colliders intersecting */
		void calculateCollisions(const IntersectionCallback& callback) const;

		/** Calculates all the Colliders that are currently intersecting with
		 * the given ray
		 *
		 * @param	rayOrigin the origin of the ray
		 * @param	rayDirection the direction of the ray
		 * @param	callback the function that must be called for each
		 *			of the Colliders intersecting with the ray */
		void calculateIntersections(
			const glm::vec3& rayOrigin, const glm::vec3& rayDirection,
			const ColliderCallback& callback
		) const;
	private:
		/** Adds a node for the given collider in the AABB Tree
		 *
		 * @param	colliderIndex the index of the Collider */
		void addNode(std::size_t colliderIndex);

		/** Remoces the node of the given collider in the AABB Tree
		 *
		 * @param	colliderIndex the index of the Collider */
		void removeNode(std::size_t colliderIndex);

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
		 * @param	nodeIndex1 the index if the first node to swap
		 * @param	nodeIndex2 the index if the second node to swap
		 * @note	the AABBs and heights won't be updated */
		void swapNodes(std::size_t nodeIndex1, std::size_t nodeIndex2);

		/** Calculates the balance of the given node
		 *
		 * @param	nodeIndex the index of the node to calculate its balance
		 * @return	the balance, >0 if the left branch is higher, <0 if it's
		 *			lower or 0 if they are the same */
		int calculateBalance(std::size_t nodeIndex);
	};

}

#endif		// COARSE_COLLISION_DETECTOR_H
