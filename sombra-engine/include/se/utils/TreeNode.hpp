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
			&& (tn1.mSibling == tn2.mSibling)
			&& (tn1.mData == tn2.mData);
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
		iterator<t> parentIt, const T& data
	) {
		return insert(parentIt, std::move(data));
	}


	template <typename T>
	template <Traversal t>
	TreeNode<T>::iterator<t> TreeNode<T>::insert(
		iterator<t> parentIt, T&& data
	) {
		auto ret = end<t>();

		if (parentIt != end<t>()) {
			auto currentChild = std::move(parentIt->mChild);
			parentIt->mChild = std::make_unique<TreeNode>(std::move(data));
			parentIt->mChild->mParent = &(*parentIt);
			parentIt->mChild->mSibling = std::move(currentChild);

			for (ret = parentIt; &(*ret) != parentIt->mChild; ++ret);
		}

		return ret;
	}


	template <typename T>
	template <Traversal t>
	TreeNode<T>::iterator<t> TreeNode<T>::erase(iterator<t> it)
	{
		auto ret = end<t>();

		if (it != end<t>()) {
			ret = it++;
			std::swap(it, ret);

			if (it->mParent->mChild == &(*it)) {
				// Update parent child with sibling
				it->mParent->mChild = std::move(it->mSibling);
			}
			else {
				// Update left sibling
				TreeNode* child = it->mParent->mChild.get();
				while (child) {
					if (child->mSibling == &(*it)) {
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


	template <typename T, bool isConst, Traversal t>
	bool operator==(
		const typename TreeNode<T>::template TNIterator<isConst, t>& it1,
		const typename TreeNode<T>::template TNIterator<isConst, t>& it2
	) {
		return (it1.mTreeNode == it2.mTreeNode)
			&& (it1.mTreeNodeQueue == it2.mTreeNodeQueue);
	}


	template <typename T, bool isConst, Traversal t>
	bool operator!=(
		const typename TreeNode<T>::template TNIterator<isConst, t>& it1,
		const typename TreeNode<T>::template TNIterator<isConst, t>& it2
	) {
		return !(it1 == it2);
	}


	template <typename T>
	template <bool isConst, Traversal t>
	TreeNode<T>::TNIterator<isConst, t>::operator
		TreeNode<T>::TNIterator<!isConst, t>() const
	{
		TNIterator<!isConst, t> ret(nullptr);

		if constexpr (isConst) {
			ret.mTreeNode = const_cast<TreeNode*>(mTreeNode);
			for (const TreeNode* node : mTreeNodeDeque) {
				ret.mTreeNodeDeque.push_back(const_cast<TreeNode*>(node));
			}
		}
		else {
			ret.mTreeNode = mTreeNode;
			for (auto& node : mTreeNodeDeque) {
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
		if (mTreeNode) {
			if (mTreeNode->mSibling) {
				mTreeNodeDeque.push_back(mTreeNode);
				mTreeNode = mTreeNode->mSibling.get();
			}
			else {
				mTreeNode = nullptr;
				while (!mTreeNodeDeque.empty()) {
					mTreeNode = mTreeNodeDeque.front();
					mTreeNodeDeque.pop_front();
					if (mTreeNode->mChild) {
						mTreeNodeDeque.push_back(mTreeNode);
						mTreeNode = mTreeNode->mChild.get();
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
		if (mTreeNode) {
			if (mTreeNode->mChild) {
				mTreeNodeDeque.push_back(mTreeNode);
				mTreeNode = mTreeNode->mChild.get();
			}
			else if (mTreeNode->mSibling) {
				mTreeNode = mTreeNode->mSibling.get();
			}
			else {
				mTreeNode = nullptr;
				while (!mTreeNodeDeque.empty()) {
					mTreeNode = mTreeNodeDeque.back();
					mTreeNodeDeque.pop_back();
					if (mTreeNode->mSibling) {
						mTreeNode = mTreeNode->mSibling.get();
						break;
					}
				}
			}
		}
	}

}

#endif		// TREE_NODE_HPP