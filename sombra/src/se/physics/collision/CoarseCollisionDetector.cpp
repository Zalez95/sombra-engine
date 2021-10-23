#include "se/physics/collision/CoarseCollisionDetector.h"
#include "se/physics/collision/Collider.h"

namespace se::physics {

	void CoarseCollisionDetector::add(Collider* collider)
	{
		auto itCollider = mColliders.emplace(ColliderData{ collider, 0 });
		std::size_t colliderIndex = itCollider.getIndex();
		addNode(colliderIndex);
	}


	void CoarseCollisionDetector::processColliders(const ColliderCallback& callback) const
	{
		for (const ColliderData& cData : mColliders) {
			callback(cData.collider);
		}
	}


	void CoarseCollisionDetector::remove(Collider* collider)
	{
		auto itCollider = std::find_if(mColliders.begin(), mColliders.end(), [&](const ColliderData& cData) {
			return cData.collider ==  collider;
		});
		if (itCollider != mColliders.end()) {
			removeNode(itCollider.getIndex());
			mColliders.erase(itCollider);
		}
	}


	void CoarseCollisionDetector::update()
	{
		for (std::size_t i = 0; i < mColliders.size(); ++i) {
			if (mColliders[i].collider->updated()) {
				removeNode(i);
				addNode(i);
			}
		}
	}


	void CoarseCollisionDetector::calculateCollisions(const IntersectionCallback& callback) const
	{
		std::vector<std::size_t> treeStack;
		std::vector<bool> traversedColliders(mColliders.size(), false);

		for (std::size_t iCollider = 0; iCollider < mColliders.size(); ++iCollider) {
			const ColliderData& colliderData = mColliders[iCollider];
			traversedColliders[iCollider] = true;

			treeStack.push_back(mRootIndex);
			while (!treeStack.empty()) {
				std::size_t nodeIndex = treeStack.back();
				treeStack.pop_back();

				if (mNodes[nodeIndex].isLeaf) {
					std::size_t iCollider2 = mNodes[nodeIndex].colliderIndex;

					if ((iCollider != iCollider2)
						&& !traversedColliders[iCollider2]
						&& overlaps(mNodes[colliderData.nodeIndex].aabb, mNodes[nodeIndex].aabb, mEpsilon)
					) {
						callback({ colliderData.collider, mColliders[iCollider2].collider });
					}
				}
				else {
					if (overlaps(mNodes[colliderData.nodeIndex].aabb, mNodes[nodeIndex].aabb, mEpsilon)) {
						treeStack.push_back(mNodes[nodeIndex].leftChild);
						treeStack.push_back(mNodes[nodeIndex].rightChild);
					}
				}
			}
		}
	}


	void CoarseCollisionDetector::calculateIntersections(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, const ColliderCallback& callback) const
	{
		std::vector<std::size_t> treeStack;
		treeStack.push_back(mRootIndex);

		while (!treeStack.empty()) {
			std::size_t nodeIndex = treeStack.back();
			treeStack.pop_back();

			if (intersects(mNodes[nodeIndex].aabb, rayOrigin, rayDirection, mEpsilon)) {
				if (mNodes[nodeIndex].isLeaf) {
					std::size_t iCollider = mNodes[nodeIndex].colliderIndex;
					callback(mColliders[iCollider].collider);
				}
				else {
					treeStack.push_back(mNodes[nodeIndex].leftChild);
					treeStack.push_back(mNodes[nodeIndex].rightChild);
				}
			}
		}
	}

// Private functions
	void CoarseCollisionDetector::addNode(std::size_t colliderIndex)
	{
		std::size_t nodeIndex = mNodes.emplace().getIndex();
		mNodes[nodeIndex].colliderIndex = colliderIndex;
		mNodes[nodeIndex].aabb = mColliders[colliderIndex].collider->getAABB();
		mColliders[colliderIndex].nodeIndex = nodeIndex;

		// If there were no nodes in the tree this one will be the new root node
		if (mNodes.size() == 1) {
			mRootIndex = nodeIndex;
			return;
		}

		// Calculate a sibling node
		std::size_t siblingIndex = calculateBestSibling(nodeIndex);

		// Insert a new parent node of the sibling and the new leaf node where
		// the sibling was
		std::size_t newParentIndex = mNodes.emplace().getIndex();
		std::size_t oldParentIndex = mNodes[siblingIndex].parent;
		mNodes[newParentIndex].isLeaf = false;
		mNodes[newParentIndex].leftChild = nodeIndex;
		mNodes[newParentIndex].rightChild = siblingIndex;
		mNodes[siblingIndex].parent = newParentIndex;
		mNodes[nodeIndex].parent = newParentIndex;

		// Update the ancestor nodes
		if (mRootIndex == siblingIndex) {
			mRootIndex = newParentIndex;
		}
		else {
			if (mNodes[oldParentIndex].leftChild == siblingIndex) {
				mNodes[oldParentIndex].leftChild = newParentIndex;
			}
			else {
				mNodes[oldParentIndex].rightChild = newParentIndex;
			}
			mNodes[newParentIndex].parent = oldParentIndex;
		}

		updateAncestors(nodeIndex);
	}


	void CoarseCollisionDetector::removeNode(std::size_t colliderIndex)
	{
		std::size_t nodeIndex = mColliders[colliderIndex].nodeIndex;

		if (nodeIndex != mRootIndex) {
			// Move the sibling node up
			std::size_t parentIndex = mNodes[nodeIndex].parent;
			std::size_t siblingIndex = (mNodes[parentIndex].leftChild == nodeIndex)?
				mNodes[parentIndex].rightChild :
				mNodes[parentIndex].leftChild;

			if (parentIndex == mRootIndex) {
				mRootIndex = siblingIndex;
			}
			else {
				std::size_t grandparentIndex = mNodes[parentIndex].parent;
				if (mNodes[grandparentIndex].leftChild == parentIndex) {
					mNodes[grandparentIndex].leftChild = siblingIndex;
				}
				else {
					mNodes[grandparentIndex].rightChild = siblingIndex;
				}
				mNodes[siblingIndex].parent = grandparentIndex;

				// Update the ancestor nodes
				updateAncestors(siblingIndex);
			}

			// Remove the parent node
			mNodes.erase(mNodes.begin().setIndex(parentIndex));
		}

		// Remove the node
		mNodes.erase(mNodes.begin().setIndex(nodeIndex));
	}


	std::size_t CoarseCollisionDetector::calculateBestSibling(std::size_t nodeIndex)
	{
		struct StackContent
		{
			std::size_t nodeIndex;
			float ancestorCost;
		};

		std::vector<StackContent> stack = { { mRootIndex, 0.0f } };
		std::size_t bestSibling = mRootIndex;
		float bestCost = std::numeric_limits<float>::max();

		while (!stack.empty()) {
			StackContent sContent = stack.back();
			stack.pop_back();

			float currentCost = calculateArea(expand(mNodes[sContent.nodeIndex].aabb, mNodes[nodeIndex].aabb)) + sContent.ancestorCost;
			if (currentCost < bestCost) {
				bestSibling = sContent.nodeIndex;
				bestCost = currentCost;
			}

			if (!mNodes[sContent.nodeIndex].isLeaf) {
				float branchCost = calculateArea(mNodes[sContent.nodeIndex].aabb) + sContent.ancestorCost;
				float traverseCost = calculateArea(mNodes[nodeIndex].aabb) + branchCost;
				if (traverseCost < bestCost) {
					stack.push_back({ mNodes[sContent.nodeIndex].leftChild, branchCost });
					stack.push_back({ mNodes[sContent.nodeIndex].rightChild, branchCost });
				}
			}
		}

		return bestSibling;
	}


	void CoarseCollisionDetector::updateAncestors(std::size_t nodeIndex)
	{
		if (nodeIndex != mRootIndex) {
			nodeIndex = mNodes[nodeIndex].parent;

			while (true) {
				rotateNodes(nodeIndex);

				std::size_t leftChild = mNodes[nodeIndex].leftChild;
				std::size_t rightChild = mNodes[nodeIndex].rightChild;
				mNodes[nodeIndex].aabb = expand(mNodes[leftChild].aabb, mNodes[rightChild].aabb);

				if (nodeIndex == mRootIndex) {
					return;
				}

				nodeIndex = mNodes[nodeIndex].parent;
			}
		}
	}


	void CoarseCollisionDetector::rotateNodes(std::size_t nodeIndex)
	{
		// Update the height
		std::size_t leftChild	= mNodes[nodeIndex].leftChild;
		std::size_t rightChild	= mNodes[nodeIndex].rightChild;
		mNodes[nodeIndex].height = std::max(mNodes[leftChild].height, mNodes[rightChild].height) + 1;

		if (nodeIndex == mRootIndex) {
			return;
		}

		std::size_t parent	= mNodes[nodeIndex].parent;
		std::size_t brother	= (mNodes[parent].leftChild == nodeIndex)?
			mNodes[parent].rightChild :
			mNodes[parent].leftChild;

		int parentBalance = calculateBalance(parent);
		if (std::abs(parentBalance) > 1) {	// Rotate
			if (mNodes[leftChild].height > mNodes[rightChild].height) {
				swapNodes(leftChild, brother);
			}
			else {
				swapNodes(rightChild, brother);
			}

			// Update the height again
			mNodes[nodeIndex].height = std::max(mNodes[leftChild].height, mNodes[rightChild].height) + 1;
		}
	}


	void CoarseCollisionDetector::swapNodes(std::size_t nodeIndex1, std::size_t nodeIndex2)
	{
		std::size_t parentIndex1 = mNodes[nodeIndex1].parent;
		std::size_t parentIndex2 = mNodes[nodeIndex2].parent;

		mNodes[nodeIndex1].parent = parentIndex2;
		if (mNodes[parentIndex2].leftChild == nodeIndex2) {
			mNodes[parentIndex2].leftChild = nodeIndex1;
		}
		else {
			mNodes[parentIndex2].rightChild = nodeIndex1;
		}

		mNodes[nodeIndex2].parent = parentIndex1;
		if (mNodes[parentIndex1].leftChild == nodeIndex1) {
			mNodes[parentIndex1].leftChild = nodeIndex2;
		}
		else {
			mNodes[parentIndex1].rightChild = nodeIndex2;
		}
	}


	int CoarseCollisionDetector::calculateBalance(std::size_t nodeIndex)
	{
		std::size_t leftChild = mNodes[nodeIndex].leftChild;
		std::size_t rightChild = mNodes[nodeIndex].rightChild;
		return static_cast<int>(mNodes[leftChild].height) - static_cast<int>(mNodes[rightChild].height);
	}

}
