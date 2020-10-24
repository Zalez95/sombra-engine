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
		auto it = begin<t>();

		while (it != end<t>()) {
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
	TreeNode<T>::iterator<t> TreeNode<T>::insert(
		const_iterator<t> parentIt, std::unique_ptr<TreeNode> descendant
	) {
		auto ret = end<t>();

		if (parentIt != cend<t>()) {
			auto parentNode = const_cast<TreeNode*>( &(*parentIt) );

			descendant->mParent = parentNode;
			auto currentChild = std::move(parentNode->mChild);
			parentNode->mChild = std::move(descendant);
			parentNode->mChild->mSibling = std::move(currentChild);

			ret = parentIt;
			do {
				++ret;
			}
			while (&(*ret) != parentIt->mChild.get());
		}

		return ret;
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
	TreeNode<T>::iterator<t> TreeNode<T>::erase(iterator<t> it)
	{
		auto ret = end<t>();

		if (it != end<t>()) {
			ret = it++;
			std::swap(it, ret);

			if (it->mParent->mChild.get() == &(*it)) {
				// Update parent child with sibling
				it->mParent->mChild = std::move(it->mSibling);
			}
			else {
				// Update left sibling
				TreeNode* child = it->mParent->mChild.get();
				while (child) {
					if (child->mSibling.get() == &(*it)) {
						child->mSibling = std::move(it->mSibling);
						child = nullptr;
					}
					else {
						child = child->mSibling.get();
					}
				}
			}
		}

		return ret;
	}


	template <typename T>
	template <bool isConst, Traversal t>
	TreeNode<T>::TNIterator<isConst, t>::TNIterator(TreeNodeType* treeNode)
	{
		if (treeNode) {
			mTreeNodeDeque = { treeNode };
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

}

#endif		// TREE_NODE_HPP
