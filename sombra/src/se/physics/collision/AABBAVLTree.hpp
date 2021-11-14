#ifndef AABB_AVL_TREE_HPP
#define AABB_AVL_TREE_HPP

namespace se::physics {

	template <typename T>
	std::size_t AABBAVLTree<T>::addNode(const AABB& aabb, const T& userData)
	{
		std::size_t nodeIndex = mNodes.emplace().getIndex();
		mNodes[nodeIndex].parent = nodeIndex;
		mNodes[nodeIndex].aabb = aabb;
		mNodes[nodeIndex].userData = userData;

		// If there were no nodes in the tree this one will be the new root node
		if (mNodes.size() == 1) {
			mRootIndex = nodeIndex;
			return nodeIndex;
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
			mNodes[newParentIndex].parent = newParentIndex;
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

		return nodeIndex;
	}


	template <typename T>
	void AABBAVLTree<T>::removeNode(std::size_t nodeId)
	{
		if (nodeId != mRootIndex) {
			// Move the sibling node up
			std::size_t parentIndex = mNodes[nodeId].parent;
			std::size_t siblingIndex = (mNodes[parentIndex].leftChild == nodeId)?
				mNodes[parentIndex].rightChild :
				mNodes[parentIndex].leftChild;

			if (parentIndex == mRootIndex) {
				mNodes[siblingIndex].parent = siblingIndex;
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
		mNodes.erase(mNodes.begin().setIndex(nodeId));
	}


	template <typename T>
	template <typename F>
	void AABBAVLTree<T>::calculateAllOverlaps(float epsilon, F&& callback) const
	{
		std::vector<std::size_t> treeStack;

		utils::PackedVector<bool> traversedNodes;
		traversedNodes.replicate(mNodes, false);

		for (auto itNode = mNodes.begin(); itNode != mNodes.end(); ++itNode) {
			std::size_t nodeIndex1 = itNode.getIndex();
			traversedNodes[nodeIndex1] = true;

			if (mNodes[nodeIndex1].isLeaf) {
				treeStack.push_back(mRootIndex);
				while (!treeStack.empty()) {
					std::size_t nodeIndex2 = treeStack.back();
					treeStack.pop_back();

					if (mNodes[nodeIndex2].isLeaf) {
						if (!traversedNodes[nodeIndex2]
							&& overlaps(mNodes[nodeIndex1].aabb, mNodes[nodeIndex2].aabb, epsilon)
						) {
							callback(nodeIndex1, nodeIndex2);
						}
					}
					else {
						if (overlaps(mNodes[nodeIndex1].aabb, mNodes[nodeIndex2].aabb, epsilon)) {
							treeStack.push_back(mNodes[nodeIndex2].leftChild);
							treeStack.push_back(mNodes[nodeIndex2].rightChild);
						}
					}
				}
			}
		}
	}


	template <typename T>
	template <typename F>
	void AABBAVLTree<T>::calculateOverlapsWith(const AABB& aabb, float epsilon, F&& callback) const
	{
		std::vector<std::size_t> treeStack = { mRootIndex };
		while (!treeStack.empty()) {
			std::size_t nodeIndex = treeStack.back();
			treeStack.pop_back();

			if (overlaps(mNodes[nodeIndex].aabb, aabb, epsilon)) {
				if (mNodes[nodeIndex].isLeaf) {
					callback(nodeIndex);
				}
				else {
					treeStack.push_back(mNodes[nodeIndex].leftChild);
					treeStack.push_back(mNodes[nodeIndex].rightChild);
				}
			}
		}
	}


	template <typename T>
	template <typename F>
	void AABBAVLTree<T>::calculateIntersectionsWith(const Ray& ray, float epsilon, F&& callback) const
	{
		std::vector<std::size_t> treeStack = { mRootIndex };
		while (!treeStack.empty()) {
			std::size_t nodeIndex = treeStack.back();
			treeStack.pop_back();

			if (intersects(mNodes[nodeIndex].aabb, ray, epsilon)) {
				if (mNodes[nodeIndex].isLeaf) {
					callback(nodeIndex);
				}
				else {
					treeStack.push_back(mNodes[nodeIndex].leftChild);
					treeStack.push_back(mNodes[nodeIndex].rightChild);
				}
			}
		}
	}

// Private functions
	template <typename T>
	std::size_t AABBAVLTree<T>::calculateBestSibling(std::size_t nodeIndex)
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


	template <typename T>
	void AABBAVLTree<T>::updateAncestors(std::size_t nodeIndex)
	{
		if (nodeIndex != mRootIndex) {
			nodeIndex = mNodes[nodeIndex].parent;

			while (true) {
				std::size_t leftChild = mNodes[nodeIndex].leftChild;
				std::size_t rightChild = mNodes[nodeIndex].rightChild;
				mNodes[nodeIndex].height = std::max(mNodes[leftChild].height, mNodes[rightChild].height) + 1;
				mNodes[nodeIndex].aabb = expand(mNodes[leftChild].aabb, mNodes[rightChild].aabb);

				rotateNodes(nodeIndex);

				if (nodeIndex == mRootIndex) {
					return;
				}

				nodeIndex = mNodes[nodeIndex].parent;
			}
		}
	}


	template <typename T>
	void AABBAVLTree<T>::rotateNodes(std::size_t nodeIndex)
	{
		if (mNodes[nodeIndex].isLeaf) {
			return;
		}

		std::size_t leftChild	= mNodes[nodeIndex].leftChild;
		std::size_t rightChild	= mNodes[nodeIndex].rightChild;

		int balance = calculateBalance(nodeIndex);
		if (balance > 1) {
			int leftChildBalance = calculateBalance(leftChild);
			if (leftChildBalance > 0) {			// Left-left rotation
				swapNodes(nodeIndex, leftChild);
			}
			else if (leftChildBalance < 0) {	// Left-right rotation
				std::size_t grandChildLR = mNodes[leftChild].rightChild;
				swapNodes(leftChild, grandChildLR);
				swapNodes(nodeIndex, grandChildLR);
			}
		}
		else if (balance < -1) {
			int rightChildBalance = calculateBalance(rightChild);
			if (rightChildBalance < 0) {		// right-right rotation
				swapNodes(nodeIndex, rightChild);
			}
			else if (rightChildBalance > 0) {	// Right-left rotation
				std::size_t grandChildRL = mNodes[rightChild].leftChild;
				swapNodes(rightChild, grandChildRL);
				swapNodes(nodeIndex, grandChildRL);
			}
		}
	}


	template <typename T>
	void AABBAVLTree<T>::swapNodes(std::size_t parent, std::size_t child)
	{
		// Move the child up
		if (mRootIndex == parent) {
			mNodes[child].parent = child;
			mRootIndex = child;
		}
		else {
			std::size_t grandparent = mNodes[parent].parent;
			if (mNodes[grandparent].leftChild == parent) {
				mNodes[grandparent].leftChild = child;
			}
			else {
				mNodes[grandparent].rightChild = child;
			}
			mNodes[child].parent = grandparent;
		}

		// Move the parent down
		if (mNodes[parent].leftChild == child) {
			std::size_t grandchild = mNodes[child].rightChild;
			mNodes[child].rightChild = parent;

			mNodes[grandchild].parent = parent;
			mNodes[parent].leftChild = grandchild;
		}
		else {
			std::size_t grandchild = mNodes[child].leftChild;
			mNodes[child].leftChild = parent;

			mNodes[grandchild].parent = parent;
			mNodes[parent].rightChild = grandchild;
		}
		mNodes[parent].parent = child;

		// Update the height and AABB of the parent and child nodes
		std::size_t iPLNode = mNodes[parent].leftChild, iPRNode = mNodes[parent].rightChild,
			iCLNode = mNodes[child].leftChild, iCRNode = mNodes[child].rightChild;
		mNodes[parent].height = std::max(mNodes[iPLNode].height, mNodes[iPRNode].height) + 1;
		mNodes[child].height = std::max(mNodes[iCLNode].height, mNodes[iCRNode].height) + 1;
		mNodes[parent].aabb = expand(mNodes[iPLNode].aabb, mNodes[iPRNode].aabb);
		mNodes[child].aabb = expand(mNodes[iCLNode].aabb, mNodes[iCRNode].aabb);
	}


	template <typename T>
	int AABBAVLTree<T>::calculateBalance(std::size_t nodeIndex)
	{
		bool isLeaf = mNodes[nodeIndex].isLeaf;
		std::size_t leftChild = mNodes[nodeIndex].leftChild;
		std::size_t rightChild = mNodes[nodeIndex].rightChild;
		return isLeaf? 0 : static_cast<int>(mNodes[leftChild].height) - static_cast<int>(mNodes[rightChild].height);
	}

}

#endif		// AABB_AVL_TREE_HPP
