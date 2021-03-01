#ifndef TREE_NODE_HPP
#define TREE_NODE_HPP

namespace se::utils {

	template <typename T>
	TreeNode<T>::TreeNode(const TreeNode& other) :
		mParent(nullptr), mData(other.mData)
	{
		if (other.mChild) {
			mChild = std::make_unique<TreeNode>(*other.mChild);

			TreeNode* child = mChild.get();
			while (child) {
				child->mParent = this;
				child = child->mSibling.get();
			}
		}

		if (other.mSibling) {
			mSibling = std::make_unique<TreeNode>(*other.mSibling);
		}
	}


	template <typename T>
	TreeNode<T>& TreeNode<T>::operator=(const TreeNode& other)
	{
		mParent = nullptr;
		mData = other.mData;

		if (other.mChild) {
			mChild = std::make_unique<TreeNode>(*other.mChild);

			TreeNode* child = mChild.get();
			while (child) {
				child->mParent = this;
				child = child->mSibling.get();
			}
		}

		if (other.mSibling) {
			mSibling = std::make_unique<TreeNode>(*other.mSibling);
		}

		return *this;
	}


	template <typename T>
	bool operator==(const TreeNode<T>& tn1, const TreeNode<T>& tn2)
	{
		return (tn1.mParent == tn2.mParent)
			&& (tn1.mChild == tn2.mChild)
			&& (tn1.mSibling == tn2.mSibling);
	}


	template <typename T>
	bool operator!=(const TreeNode<T>& tn1, const TreeNode<T>& tn2)
	{
		return !(tn1 == tn2);
	}


	template <typename T>
	template <Traversal t>
	typename TreeNode<T>::size_type TreeNode<T>::size() const
	{
		size_type count = 0;
		for (auto it = begin<t>(); it != end<t>(); ++it) {
			++count;
		}
		return count;
	}


	template <typename T>
	template <Traversal t>
	TreeNode<T>::iterator<t> TreeNode<T>::find(const T& data)
	{
		const TreeNode<T>* cThis = this;
		return cThis->find<t>(data);
	}


	template <typename T>
	template <Traversal t>
	TreeNode<T>::const_iterator<t> TreeNode<T>::find(const T& data) const
	{
		auto it = cbegin<t>();

		while (it != cend<t>()) {
			if (it->mData == data) {
				break;
			}
			else {
				++it;
			}
		}

		return it;
	}


	template <typename T>
	template <Traversal t>
	TreeNode<T>::iterator<t> TreeNode<T>::find(const TreeNode& node)
	{
		const TreeNode<T>* cThis = this;
		return cThis->find<t>(node);
	}


	template <typename T>
	template <Traversal t>
	TreeNode<T>::const_iterator<t> TreeNode<T>::find(const TreeNode& node) const
	{
		auto it = cbegin<t>();

		while (it != cend<t>()) {
			if (&(*it) == &node) {
				break;
			}
			else {
				++it;
			}
		}

		return it;
	}


	template <typename T>
	template <Traversal t>
	TreeNode<T>::iterator<t> TreeNode<T>::insert(
		const_iterator<t> parentIt, std::unique_ptr<TreeNode> descendant
	) {
		auto parentNode = this;
		if (parentIt != cend<t>()) {
			parentNode = const_cast<TreeNode*>( &(*parentIt) );
		}

		TreeNode& descendantRef = *descendant;

		descendant->mParent = parentNode;
		if (parentNode->mChild) {
			TreeNode* lastChild = parentNode->mChild.get();
			while (lastChild->mSibling) {
				lastChild = lastChild->mSibling.get();
			}

			lastChild->mSibling = std::move(descendant);
		}
		else {
			parentNode->mChild = std::move(descendant);
		}

		return find(descendantRef);
	}


	template <typename T>
	template <Traversal t, typename... Args>
	TreeNode<T>::iterator<t> TreeNode<T>::emplace(
		const_iterator<t> parentIt, Args&&... args
	) {
		auto descendant = std::make_unique<TreeNode>(std::forward<Args>(args)...);
		return insert(parentIt, std::move(descendant));
	}


	template <typename T>
	template <Traversal t>
	TreeNode<T>::iterator<t> TreeNode<T>::erase(iterator<t> it, bool eraseDescendants)
	{
		TreeNode* currentNode = &(*it);
		if (eraseDescendants) {
			mChild = nullptr;
		}
		else {
			moveChildNodesUp(currentNode);
		}

		++it;
		removeNode(currentNode);
		return it;
	}


	template <typename T>
	template <Traversal t>
	TreeNode<T>::iterator<t> TreeNode<T>::move(iterator<t> it, const_iterator<t> parentIt, bool moveDescendants)
	{
		TreeNode* currentNode = &(*it);
		if (!moveDescendants) {
			moveChildNodesUp(currentNode);
		}

		std::unique_ptr<TreeNode> nodeUPtr = removeNode(currentNode);
		return insert(parentIt, std::move(nodeUPtr));
	}


	template <typename T>
	void TreeNode<T>::moveChildNodesUp(TreeNode* node)
	{
		TreeNode* lastChild = nullptr;
		TreeNode* currentChild = node->mChild.get();
		while (currentChild) {
			currentChild->mParent = node->mParent;
			lastChild = currentChild;
			currentChild = currentChild->mSibling.get();
		}

		if (lastChild) {
			lastChild->mSibling = std::move(node->mSibling);
			node->mSibling = std::move(node->mChild);
		}
	}


	template <typename T>
	std::unique_ptr<TreeNode<T>> TreeNode<T>::removeNode(TreeNode* node)
	{
		std::unique_ptr<TreeNode<T>> ret;

		if (node->mParent->mChild.get() == node) {
			ret = std::move(node->mParent->mChild);
			node->mParent->mChild = std::move(ret->mSibling);
		}
		else {
			TreeNode* parentChild = node->mParent->mChild.get();
			while (parentChild) {
				if (parentChild->mSibling.get() == node) {
					ret = std::move(parentChild->mSibling);
					parentChild->mSibling = std::move(ret->mSibling);
					break;
				}
				parentChild = parentChild->mSibling.get();
			}
		}

		ret->mParent = nullptr;
		return ret;
	}


	template <typename T>
	template <bool isConst, Traversal t>
	TreeNode<T>::TNIterator<isConst, t>::TNIterator(TreeNodeType* treeNode)
	{
		if (treeNode) {
			if constexpr ((t == Traversal::BFS) || (t == Traversal::DFSPreOrder)) {
				if (treeNode->mChild) {
					mTreeNodeDeque = { treeNode->mChild.get() };
				}
			}
			else {
				treeNode = treeNode->mChild.get();
				while (treeNode) {
					mTreeNodeDeque.push_back(treeNode);
					treeNode = treeNode->mChild.get();
				}
			}
		}
	}


	template <typename T>
	template <bool isConst, Traversal t>
	TreeNode<T>::TNIterator<isConst, t>::operator
		TreeNode<T>::TNIterator<!isConst, t>() const
	{
		TNIterator<!isConst, t> ret(nullptr);

		if constexpr (isConst) {
			for (const TreeNode* node : mTreeNodeDeque) {
				ret.mTreeNodeDeque.push_back(const_cast<TreeNode*>(node));
			}
		}
		else {
			for (TreeNode* node : mTreeNodeDeque) {
				ret.mTreeNodeDeque.push_back(node);
			}
		}

		return ret;
	}


	template <typename T>
	template <bool isConst, Traversal t>
	TreeNode<T>::TNIterator<isConst, t>&
		TreeNode<T>::TNIterator<isConst, t>::operator++()
	{
		if constexpr (t == Traversal::BFS) {
			nextBFS();
		}
		else if constexpr (t == Traversal::DFSPreOrder) {
			nextDFSPreOrder();
		}
		else if constexpr (t == Traversal::DFSPostOrder) {
			nextDFSPostOrder();
		}
		else if constexpr (t == Traversal::Children) {
			nextChildren();
		}

		return *this;
	}


	template <typename T>
	template <bool isConst, Traversal t>
	TreeNode<T>::TNIterator<isConst, t>
		TreeNode<T>::TNIterator<isConst, t>::operator++(int)
	{
		TNIterator ret(*this);
		operator++();
		return ret;
	}


	template <typename T>
	template <bool isConst, Traversal t>
	void TreeNode<T>::TNIterator<isConst, t>::nextBFS()
	{
		if (!mTreeNodeDeque.empty()) {
			TreeNodeType* node = mTreeNodeDeque.back();
			if (node->mSibling) {
				mTreeNodeDeque.push_back(node->mSibling.get());
			}
			else {
				while (!mTreeNodeDeque.empty()) {
					node = mTreeNodeDeque.front();
					mTreeNodeDeque.pop_front();
					if (node->mChild) {
						mTreeNodeDeque.push_back(node->mChild.get());
						break;
					}
				}
			}
		}
	}


	template <typename T>
	template <bool isConst, Traversal t>
	void TreeNode<T>::TNIterator<isConst, t>::nextDFSPreOrder()
	{
		if (!mTreeNodeDeque.empty()) {
			TreeNodeType* node = mTreeNodeDeque.back();
			if (node->mChild) {
				mTreeNodeDeque.push_back(node->mChild.get());
			}
			else {
				while (!mTreeNodeDeque.empty()) {
					node = mTreeNodeDeque.back();
					mTreeNodeDeque.pop_back();
					if (node->mSibling) {
						mTreeNodeDeque.push_back(node->mSibling.get());
						break;
					}
				}
			}
		}
	}


	template <typename T>
	template <bool isConst, Traversal t>
	void TreeNode<T>::TNIterator<isConst, t>::nextDFSPostOrder()
	{
		if (!mTreeNodeDeque.empty()) {
			TreeNodeType* node = mTreeNodeDeque.back();
			mTreeNodeDeque.pop_back();
			if (node->mSibling) {
				node = node->mSibling.get();
				while (node) {
					mTreeNodeDeque.push_back(node);
					node = node->mChild.get();
				}
			}
		}
	}


	template <typename T>
	template <bool isConst, Traversal t>
	void TreeNode<T>::TNIterator<isConst, t>::nextChildren()
	{
		if (!mTreeNodeDeque.empty()) {
			TreeNodeType* node = mTreeNodeDeque.back();
			mTreeNodeDeque.pop_back();
			if (node->mSibling) {
				mTreeNodeDeque.push_back(node->mSibling.get());
			}
		}
	}

}

#endif		// TREE_NODE_HPP
