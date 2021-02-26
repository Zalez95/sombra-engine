#ifndef TREE_NODE_H
#define TREE_NODE_H

#include <deque>
#include <memory>

namespace se::utils {

	/** The algorithms to use for visiting the TreeNodes */
	enum class Traversal { BFS, DFSPreOrder, DFSPostOrder };


	/**
	 * Class TreeNode, it's a hierarchical data structure in which each TreeNode
	 * holds part of the data. A tree is a kind of acyclic graph with just one
	 * root node. Each node of the tree can be seen as a subtree, and its
	 * represented in a parent-child-sibling structure for minimizing the memory
	 * storage requirements.
	 * The TreeNode also has functions for iterating it with different
	 * algorithms and also finding Nodes by the data that each hold
	 */
	template <typename T>
	class TreeNode
	{
	public:		// Nested types
		using value_type = T;
		using size_type = std::size_t;
		template <bool isConst, Traversal t> class TNIterator;
		template <Traversal t> using iterator = TNIterator<false, t>;
		template <Traversal t> using const_iterator = TNIterator<true, t>;

	private:	// Attributes
		/** A pointer to the parent TreeNode of the current one */
		TreeNode* mParent = nullptr;

		/** A pointer to the child TreeNode of the current one */
		std::unique_ptr<TreeNode> mChild;

		/** A pointer to the sibling TreeNode of the current one */
		std::unique_ptr<TreeNode> mSibling;

		/** The data of the TreeNode */
		T mData;

	public:		// Functions
		/** Creates a new TreeNode */
		TreeNode() = default;

		/** Creates a new TreeNode
		 *
		 * @param	data the data of the TreeNode */
		TreeNode(const T& data) : mData(data) {};

		/** Creates a new TreeNode
		 *
		 * @param	data the data of the TreeNode */
		TreeNode(T&& data) : mData(std::move(data)) {};

		/** Copy constructor
		 *
		 * @param	other the other TreeNode to copy */
		TreeNode(const TreeNode& other);
		TreeNode(TreeNode&& other) = default;

		/** Class destructor */
		~TreeNode() = default;

		/** Assignment operator
		 *
		 * @param	other the other TreeNode to copy */
		TreeNode& operator=(const TreeNode& other);
		TreeNode& operator=(TreeNode&& other) = default;

		/** Compares the given TreeNodes
		 *
		 * @param	tn1 the first TreeNode to compare
		 * @param	tn2 the second TreeNode to compare
		 * @return	true if both TreeNodes are equal, false otherwise */
		template <typename U>
		friend bool operator==(const TreeNode<U>& tn1, const TreeNode<U>& tn2);

		/** Compares the given TreeNodes
		 *
		 * @param	tn1 the first TreeNode to compare
		 * @param	tn2 the second TreeNode to compare
		 * @return	true if both TreeNodes are different, false otherwise */
		template <typename U>
		friend bool operator!=(const TreeNode<U>& tn1, const TreeNode<U>& tn2);

		/** @return	the initial iterator of the TreeNode descendants */
		template <Traversal t = Traversal::BFS>
		iterator<t> begin() { return iterator<t>(this); }

		/** @return	the initial iterator of the TreeNode descendants */
		template <Traversal t = Traversal::BFS>
		const_iterator<t> cbegin() const { return const_iterator<t>(this); }

		/** @return	the final iterator of the TreeNode */
		template <Traversal t = Traversal::BFS>
		iterator<t> end() { return iterator<t>(); }

		/** @return	the final iterator of the TreeNode */
		template <Traversal t = Traversal::BFS>
		const_iterator<t> cend() const { return const_iterator<t>(); }

		/** @return	true if the current TreeNode is a leaf, false otherwise */
		bool isLeaf() const { return mChild == nullptr; };

		/** @return	a pointer to the parent TreeNode of the current one */
		TreeNode* getParent() { return mParent; };

		/** @return	a pointer to the parent TreeNode of the current one */
		const TreeNode* getParent() const { return mParent; };

		/** @return	a pointer to the child TreeNode of the current one */
		TreeNode* getChild() { return mChild.get(); };

		/** @return	a pointer to the child TreeNode of the current one */
		const TreeNode* getChild() const { return mChild.get(); };

		/** @return	a pointer to the sibling TreeNode of the current one */
		TreeNode* getSibling() { return mSibling.get(); };

		/** @return	a pointer to the sibling TreeNode of the current one */
		const TreeNode* getSibling() const { return mSibling.get(); };

		/** @return	the data of the TreeNode */
		T& getData() { return mData; };

		/** @return	the data of the TreeNode */
		const T& getData() const { return mData; };

		/** @return	the number of TreeNodes in the current tree
		 *			(current node + descendants) */
		template <Traversal t = Traversal::BFS>
		size_type size() const;

		/** Searchs a descendant TreeNode with the same data than the given one
		 *
		 * @param	data the data to search in the TreeNode
		 * @return	an iterator to the TreeNode where the data has been found,
		 *			or to end if it wasn't found */
		template <Traversal t = Traversal::BFS>
		iterator<t> find(const T& data);

		/** Searchs a descendant TreeNode with the same data than the given one
		 *
		 * @param	data the data to search in the TreeNode
		 * @return	a const_iterator to the TreeNode where the data has been
		 *			found, or to end if it wasn't found */
		template <Traversal t = Traversal::BFS>
		const_iterator<t> find(const T& data) const;

		/** Searchs a descendant TreeNode
		 *
		 * @param	node the TreeNode to search
		 * @return	an iterator to the TreeNode or to end if it wasn't found */
		template <Traversal t = Traversal::BFS>
		iterator<t> find(const TreeNode& node);

		/** Searchs a descendant TreeNode
		 *
		 * @param	node the TreeNode to search
		 * @return	a const_iterator to the TreeNode or to end if it wasn't
		 *			found */
		template <Traversal t = Traversal::BFS>
		const_iterator<t> find(const TreeNode& node) const;

		/** Adds a descendant TreeNode as a child of the TreeNode pointed by
		 * the given iterator
		 *
		 * @param	parentIt an iterator to the parent TreeNode of the one to
		 *			add. If it's the end iterator, the node will be inserted as
		 *			a child of the current TreeNode
		 * @param	descendant a pointer to the TreeNode to insert
		 * @return	an iterator to the new TreeNode, end if the TreeNode
		 *			couldn't be added
		 * @note	if the parentIt isn't valid the TreeNode won't be added */
		template <Traversal t = Traversal::BFS>
		iterator<t> insert(
			const_iterator<t> parentIt,
			std::unique_ptr<TreeNode> descendant
		);

		/** Adds a descendant TreeNode as a child of the TreeNode pointed by
		 * the given iterator
		 *
		 * @param	parentIt an iterator to the parent TreeNode of the one to
		 *			add. If it's the end iterator, the node will be inserted as
		 *			a child of the current TreeNode
		 * @param	args the arguments needed for calling the constructor of
		 *			the new Element
		 * @return	an iterator to the new TreeNode, end if the TreeNode
		 *			couldn't be added
		 * @note	if the parentIt isn't valid the TreeNode won't be added */
		template <Traversal t = Traversal::BFS, typename... Args>
		iterator<t> emplace(const_iterator<t> parentIt, Args&&... args);

		/** Removes the TreeNode pointed by the given iterator
		 *
		 * @param	it an iterator to the TreeNode to remove
		 * @param	eraseDescendants if the descendant TreeNodes must be erased
		 *			or not
		 * @return	an iterator to the following TreeNode of the one erased */
		template <Traversal t = Traversal::BFS>
		iterator<t> erase(iterator<t> it, bool eraseDescendants = false);

		/** Moves the given TreeNode to another position in the hierarchy
		 *
		 * @param	it an iterator to the TreeNode to move
		 * @param	parentIt an iterator to the new parent TreeNode. If it's
		 *			the end iterator, the node will be moved as child of the
		 *			current TreeNode
		 * @param	moveDescendants if the descendant TreeNodes must be moved
		 *			or not
		 * @return	an iterator to the TreeNode moved */
		template <Traversal t = Traversal::BFS>
		iterator<t> move(
			iterator<t> it, const_iterator<t> parentIt,
			bool moveDescendants = false
		);
	private:
		/** Removes the given node and returns it
		 *
		 * @param	node a pointer to the node to remove
		 * @return	the removed node */
		std::unique_ptr<TreeNode> removeNode(TreeNode* node);

		/** Sets the first node and its siblings as children of the given
		 * parent TreeNode
		 *
		 * @param	first the pointer of the first TreeNode
		 * @param	parent the new parent TreeNode */
		void setNodesParent(std::unique_ptr<TreeNode> first, TreeNode* parent);
	};


	/**
	 * Class TNIterator, the class used to iterate through the
	 * descendant TreeNodes of a TreeNode. If a node is removed/added while
	 * iterating, the iterator will be invalidated
	 */
	template <typename T>
	template <bool isConst, Traversal t>
	class TreeNode<T>::TNIterator
	{
	public:		// Nested types
		template <bool isConst2, Traversal t2>
		friend class TNIterator;

		using TreeNodeType = std::conditional_t<isConst,
			const TreeNode, TreeNode
		>;

		using difference_type	= long;
		using value_type		= TreeNodeType;
		using pointer			= value_type*;
		using reference			= value_type&;
		using iterator_category = std::forward_iterator_tag;

	private:	// Attributes
		/** The deque needed for traversing the TreeNodes */
		std::deque<TreeNodeType*> mTreeNodeDeque;

	public:		// Functions
		/** Creates a new TNIterator located at the given TreeNode
		 *
		 * @param	treeNode a pointer to the TreeNode of the iterator */
		TNIterator(TreeNodeType* treeNode = nullptr);

		/** Implicit conversion operator between const TNIterator and non
		 * const TNIterator
		 *
		 * @return	the new TNIterator with a different template const
		 *			type */
		operator TNIterator<!isConst, t>() const;

		/** @return	a reference to the current TreeNode that the iterator is
		 *			pointing at */
		reference operator*() const { return *mTreeNodeDeque.back(); };

		/** @return	a pointer to the current TreeNode that the iterator is
		 *			pointing at */
		pointer operator->() const { return mTreeNodeDeque.back(); };

		/** Compares the given TNIterators
		 *
		 * @param	it1 the first TNIterator to compare
		 * @param	it2 the second TNIterator to compare
		 * @return	true if both iterators are equal, false otherwise */
		friend bool operator==(const TNIterator& it1, const TNIterator& it2)
		{ return (it1.mTreeNodeDeque == it2.mTreeNodeDeque); };

		/** Compares the given TNIterators
		 *
		 * @param	it1 the first TNIterator to compare
		 * @param	it2 the second TNIterator to compare
		 * @return	true if both iterators are different, false otherwise */
		friend bool operator!=(const TNIterator& it1, const TNIterator& it2)
		{ return !(it1 == it2); };

		/** Preincrement operator
		 *
		 * @return	a reference to the current iterator after its
		 *			incrementation */
		TNIterator& operator++();

		/** Postincrement operator
		 *
		 * @return	a copy of the current iterator with the previous value
		 *			to the incrementation */
		TNIterator operator++(int);
	private:
		/** Calculates the next TreeNode to point using the BFS algorithm */
		void nextBFS();

		/** Calculates the next TreeNode to point using the DFS pre-order
		 * algorithm */
		void nextDFSPreOrder();

		/** Calculates the next TreeNode to point using the DFS post-order
		 * algorithm */
		void nextDFSPostOrder();
	};

}

#include "TreeNode.hpp"

#endif		// TREE_NODE_H
