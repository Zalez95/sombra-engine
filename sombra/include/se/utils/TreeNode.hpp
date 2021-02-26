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
		setNodesParent(std::move(descendant), parentNode);
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
		auto nextIt = ++iterator<t>(it);
		auto parent = it->mParent;

		std::unique_ptr<TreeNode> nodeUPtr = removeNode(&(*it));
		if (!eraseDescendants) {
			setNodesParent(std::move(nodeUPtr->mChild), parent);
		}

		iterator<t> ret;
		if (nextIt != end<t>()) {
			ret = find(*nextIt);
		}
		return ret;
	}


	template <typename T>
	template <Traversal t>
	TreeNode<T>::iterator<t> TreeNode<T>::move(iterator<t> it, const_iterator<t> parentIt, bool moveDescendants)
	{
		TreeNode* oldParent = it->mParent;

		std::unique_ptr<TreeNode> nodeUPtr = removeNode(&(*it));
		if (!moveDescendants) {
			setNodesParent(std::move(nodeUPtr->mChild), oldParent);
		}

		return insert(parentIt, std::move(nodeUPtr));
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
	void TreeNode<T>::setNodesParent(std::unique_ptr<TreeNode> first, TreeNode* parent)
	{
		TreeNode* child = first.get();
		while (child) {
			child->mParent = parent;
			child = child->mSibling.get();
		}

		TreeNode* parentChild = parent->mChild.get();
		if (parentChild) {
			while (parentChild->mSibling) {
				parentChild = parentChild->mSibling.get();
			}

			parentChild->mSibling = std::move(first);
		}
		else {
			parent->mChild = std::move(first);
		}
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

}

#endif		// TREE_NODE_HPP
