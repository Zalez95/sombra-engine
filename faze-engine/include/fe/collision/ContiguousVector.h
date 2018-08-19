#ifndef CONTIGUOUS_VECTOR_H
#define CONTIGUOUS_VECTOR_H

#include <set>
#include <vector>
#include <iterator>

namespace fe { namespace collision {

	template<class T, bool isConst> class ContiguousVectorIterator;


	/**
	 * Class ContiguousVector, it works as an usual vector but it also caches
	 * the released elements instead of erasing them for preventing the old
	 * indices pointing to the vector from being invalidated.
	 *
	 * @note	it doesn't prevent from pointer invalidations due to the
	 *			increment of the vector size with new allocations, also the
	 *			released elements will be reused in the following allocations
	 */
	template<class T>
	class ContiguousVector
	{
	public:		// Nested types
		using iterator = ContiguousVectorIterator<T, false>;
		using const_iterator = ContiguousVectorIterator<T, true>;

	private:	// Attributes
		friend class ContiguousVectorIterator<T, false>;
		friend class ContiguousVectorIterator<T, true>;

		/** The raw data of the ContiguousVector */
		std::vector<T> mElements;

		/** The number of non free Elements of the ContiguousVector */
		std::size_t mNumElements;

		/** The indices to the freed Elements of the ContiguousVector */
		std::set<std::size_t> mFreeIndices;

	public:		// Functions
		/** Creates a new ContiguousVector */
		ContiguousVector() : mNumElements(0) {};

		/** Class destructor */
		~ContiguousVector() {};

		/** @return	the number of Elements in the ContiguousVector */
		std::size_t size() const { return mNumElements; };

		/** Returns the Element i of the ContiguousVector
		 *
		 * @param	i the index of the Element
		 * @return	a reference to the Element */
		T& operator[](std::size_t i) { return mElements[i]; };

		/** Returns the Element i of the ContiguousVector
		 *
		 * @param	i the index of the Element
		 * @return	a const reference to the Element */
		const T& operator[](std::size_t i) const { return mElements[i]; };

		/** @return	the initial iterator of the ContiguousVector */
		iterator begin() { return iterator(this); };

		/** @return	the initial iterator of the ContiguousVector */
		const_iterator begin() const { return const_iterator(this); };

		/** @return	the final iterator of the ContiguousVector */
		iterator end() { return iterator(this, mElements.size()); };

		/** @return	the final iterator of the ContiguousVector */
		const_iterator end() const
		{ return const_iterator(this, mElements.size()); };

		/** Creates a new Element in the vector or reuses an already released
		 * one
		 *
		 * @return	the index of the element */
		std::size_t create();

		/** Marks the Element located at the given index as released for
		 * future use
		 *
		 * @param	i the index of the Element
		 * @note	by releasing the elements instead than erasing them we
		 *			don't have to iterate through the elements for fixing
		 *			its indices */
		void release(std::size_t i);

		/** Checks if the Element given located at the given index is valid and
		 * active
		 *
		 * @param	i the index of the Element
		 * @return	true if is valid and active false otherwise */
		bool isActive(std::size_t i) const;
	};


	/**
	 * Class ContiguousVectorIterator, the class used to iterate through the
	 * elements of a ContiguousVector
	 */
	template<class T, bool isConst = false>
	class ContiguousVectorIterator : public std::iterator<
		std::bidirectional_iterator_tag, T
	> {
	private:	// Attributes
		using ElementType = typename std::conditional_t<isConst, const T, T>;
		using VectorType = typename std::conditional_t<isConst,
			const ContiguousVector<T>, ContiguousVector<T>
		>;

		/** A pointer to the vector to iterate */
		VectorType* mVector;

		/** The current position in the vector */
		std::size_t mIndex;

	public:		// Functions
		/** Creates a new ContiguousVectorIterator located at the initial valid
		 * position of the given ContiguousVector (begin)
		 *
		 * @param	vector a pointer to the vector to iterate */
		ContiguousVectorIterator(VectorType* vector);

		/** Creates a new ContiguousVectorIterator
		 *
		 * @param	vector a pointer to the vector to iterate
		 * @param	index the inital index of the iterator */
		ContiguousVectorIterator(VectorType* vector, std::size_t index) :
			mVector(vector), mIndex(index) {};

		/** Class destructor */
		~ContiguousVectorIterator() {};

		/** @return	the index of the Element that the iterator is pointing to */
		std::size_t getIndex() const { return mIndex; };

		/** @return	a reference to the current Element that the iterator is
		 *			pointing at */
		ElementType& operator*() { return (*mVector)[mIndex]; };

		/** @return	a pointer to the current Element that the iterator is
		 *			pointing at */
		ElementType* operator->() { return &(*mVector)[mIndex]; };

		/** Compares the current iterator with the given one
		 *
		 * @param	other the other iterator to compare
		 * @return	true if both iterators are equal, false otherwise */
		bool operator==(const ContiguousVectorIterator& other) const
		{ return (mVector == other.mVector) && (mIndex == other.mIndex); };

		/** Compares the current iterator with the given one
		 *
		 * @param	other the other iterator to compare
		 * @return	true if both iterators are different, false otherwise */
		bool operator!=(const ContiguousVectorIterator& other) const
		{ return !(*this == other); };

		/** Preincrement operator
		 *
		 * @return	a reference to the current iterator after its
		 *			incrementation */
		ContiguousVectorIterator& operator++();

		/** Postincrement operator
		 *
		 * @return	a copy of the current iterator with the previous value to
		 *			the incrementation */
		ContiguousVectorIterator operator++(int);

		/** Predecrement operator
		 *
		 * @return	a reference to the current iterator after its
		 *			decrementation */
		ContiguousVectorIterator& operator--();

		/** Postdecrement operator
		 *
		 * @return	a copy of the current iterator with the previous value to
		 *			the decrementation */
		ContiguousVectorIterator operator--(int);
	};

// Template functions definition
	template<class T>
	std::size_t ContiguousVector<T>::create()
	{
		std::size_t index;
		if (mFreeIndices.empty()) {
			mElements.emplace_back();
			index = mElements.size() - 1;
		}
		else {
			auto it = mFreeIndices.begin();
			index = *it;
			mFreeIndices.erase(it);
		}
		mNumElements++;

		return index;
	}


	template<class T>
	void ContiguousVector<T>::release(std::size_t i)
	{
		if (isActive(i)) {
			mFreeIndices.insert(i);
			mElements[i] = T();
			mNumElements--;
		}
	}


	template<class T>
	bool ContiguousVector<T>::isActive(std::size_t i) const
	{
		return (i < mElements.size())
			&& (mFreeIndices.find(i) == mFreeIndices.end());
	}


	template<class T, bool isConst>
	ContiguousVectorIterator<T, isConst>::ContiguousVectorIterator(
		VectorType* vector
	) : mVector(vector), mIndex(0)
	{
		if (!mVector->isActive(mIndex) && (mVector->mNumElements > 0)) {
			operator++();
		}
	}


	template<class T, bool isConst>
	ContiguousVectorIterator<T, isConst>&
		ContiguousVectorIterator<T, isConst>::operator++()
	{
		do {
			mIndex++;
		}
		while (
			!mVector->isActive(mIndex)
			&& (mIndex < mVector->mElements.size())
		);

		return *this;
	}


	template<class T, bool isConst>
	ContiguousVectorIterator<T, isConst>
		ContiguousVectorIterator<T, isConst>::operator++(int)
	{
		ContiguousVectorIterator<T, isConst> ret(*this);
		operator++();
		return ret;
	}


	template<class T, bool isConst>
	ContiguousVectorIterator<T, isConst>&
		ContiguousVectorIterator<T, isConst>::operator--()
	{
		do {
			mIndex--;
		}
		while (
			!mVector->isActive(mIndex)
			&& (mIndex < mVector->mElements.size())
		);

		return *this;
	}


	template<class T, bool isConst>
	ContiguousVectorIterator<T, isConst>
		ContiguousVectorIterator<T, isConst>::operator--(int)
	{
		ContiguousVectorIterator<T, isConst> ret(*this);
		operator--();
		return ret;
	}

}}

#endif		// CONTIGUOUS_VECTOR_H
