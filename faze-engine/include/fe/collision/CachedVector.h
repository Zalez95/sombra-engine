#ifndef CACHED_VECTOR_H
#define CACHED_VECTOR_H

#include <set>
#include <vector>
#include <iterator>

namespace fe { namespace collision {

	template<class T, bool isConst> class CachedVectorIterator;


	/**
	 * Class CachedVector, it works as an usual vector but it also caches the
	 * released elements instead of erasing them for preventing the old indices
	 * pointing to the vector from being invalidated.
	 *
	 * @note	it doesn't prevent from pointer invalidations due to the
	 *			increment of the vector size with new allocations, also the
	 *			released elements will be reused in the following allocations
	 */
	template<class T>
	class CachedVector
	{
	public:		// Nested types
		using iterator = CachedVectorIterator<T, false>;
		using const_iterator = CachedVectorIterator<T, true>;

	private:	// Attributes
		friend class CachedVectorIterator<T, false>;
		friend class CachedVectorIterator<T, true>;

		/** The raw data of the CachedVector */
		std::vector<T> mElements;

		/** The number of non free Elements of the Vector */
		std::size_t mNumElements;

		/** The indices to the freed Elements of the Vector */
		std::set<std::size_t> mFreeIndices;

	public:		// Functions
		/** Creates a new CachedVector */
		CachedVector() : mNumElements(0) {};

		/** Class destructor */
		~CachedVector() {};

		/** @return	the number of Elements in the CachedVector */
		std::size_t size() const { return mNumElements; };

		/** Returns the Element i of the CachedVector
		 *
		 * @param	i the index of the Element
		 * @return	a reference to the Element */
		T& operator[](std::size_t i) { return mElements[i]; };

		/** Returns the Element i of the CachedVector
		 *
		 * @param	i the index of the Element
		 * @return	a const reference to the Element */
		const T& operator[](std::size_t i) const { return mElements[i]; };

		/** @return	the initial iterator of the CachedVector */
		iterator begin() { return iterator(*this); };

		/** @return	the initial iterator of the CachedVector */
		const_iterator begin() const { return const_iterator(*this); };

		/** @return	the final iterator of the CachedVector */
		iterator end() { return iterator(*this, mElements.size()); };

		/** @return	the final iterator of the CachedVector */
		const_iterator end() const
		{ return const_iterator(*this, mElements.size()); };

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
		void free(std::size_t i);

		/** Checks if the Element given located at the given index is valid and
		 * active
		 *
		 * @param	i the index of the Element
		 * @return	true if is valid and active false otherwise */
		bool isActive(std::size_t i) const;
	};


	/**
	 * Class CachedVectorIterator, the class used to iterate through the
	 * elements of a CachedVector
	 */
	template<class T, bool isConst = false>
	class CachedVectorIterator : public std::iterator<
		std::bidirectional_iterator_tag, T
	> {
	private:	// Attributes
		using ElementType = typename std::conditional_t<isConst, const T, T>;
		using VectorType = typename std::conditional_t<isConst,
			const CachedVector<T>, CachedVector<T>
		>;

		/** The vector to iterate */
		VectorType& mVector;

		/** The current position in the vector */
		std::size_t mIndex;

	public:		// Functions
		/** Creates a new CachedVectorIterator located at the initial valid
		 * position of the given CachedVector (begin)
		 *
		 * @param	vector the vector to iterate */
		CachedVectorIterator(VectorType& vector);

		/** Creates a new CachedVectorIterator
		 *
		 * @param	vector the vector to iterate
		 * @param	index the inital index of the iterator */
		CachedVectorIterator(VectorType& vector, std::size_t index) :
			mVector(vector), mIndex(index) {};

		/** Class destructor */
		~CachedVectorIterator() {};

		/** @return	the index of the Element that the iterator is pointing to */
		std::size_t getIndex() const { return mIndex; };

		/** @return	a reference to the current Element that the iterator is
		 *			pointing at */
		ElementType& operator*() { return mVector[mIndex]; };

		/** @return	a pointer to the current Element that the iterator is
		 *			pointing at */
		ElementType* operator->() { return &mVector[mIndex]; };

		/** Compares the current iterator with the given one
		 *
		 * @param	other the other iterator to compare
		 * @return	true if both iterators are equal, false otherwise */
		bool operator==(const CachedVectorIterator& other) const
		{ return (&mVector == &other.mVector) && (mIndex == other.mIndex); };

		/** Compares the current iterator with the given one
		 *
		 * @param	other the other iterator to compare
		 * @return	true if both iterators are different, false otherwise */
		bool operator!=(const CachedVectorIterator& other) const
		{ return !(*this == other); };

		/** Preincrement operator
		 *
		 * @return	a reference to the current iterator after its
		 *			incrementation */
		CachedVectorIterator& operator++();

		/** Postincrement operator
		 *
		 * @return	a copy of the current iterator with the previous value to
		 *			the incrementation */
		CachedVectorIterator operator++(int);

		/** Predecrement operator
		 *
		 * @return	a reference to the current iterator after its
		 *			decrementation */
		CachedVectorIterator& operator--();

		/** Postdecrement operator
		 *
		 * @return	a copy of the current iterator with the previous value to
		 *			the decrementation */
		CachedVectorIterator operator--(int);
	};

// Template functions definition
	template<class T>
	std::size_t CachedVector<T>::create()
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
	void CachedVector<T>::free(std::size_t i)
	{
		if (mFreeIndices.find(i) == mFreeIndices.end()) {
			mFreeIndices.insert(i);
			mElements[i] = T();
			mNumElements--;
		}
	}


	template<class T>
	bool CachedVector<T>::isActive(std::size_t i) const
	{
		return (i < mElements.size())
			&& (mFreeIndices.find(i) == mFreeIndices.end());
	}


	template<class T, bool isConst>
	CachedVectorIterator<T, isConst>::CachedVectorIterator(VectorType& vector) :
		mVector(vector), mIndex(0)
	{
		if (!mVector.isActive(mIndex) && (mVector.mNumElements > 0)) {
			operator++();
		}
	}


	template<class T, bool isConst>
	CachedVectorIterator<T, isConst>&
		CachedVectorIterator<T, isConst>::operator++()
	{
		do {
			mIndex++;
		}
		while (
			!mVector.isActive(mIndex)
			&& (mIndex < mVector.mElements.size())
		);

		return *this;
	}


	template<class T, bool isConst>
	CachedVectorIterator<T, isConst>
		CachedVectorIterator<T, isConst>::operator++(int)
	{
		CachedVectorIterator<T> ret(*this);
		operator++();
		return ret;
	}


	template<class T, bool isConst>
	CachedVectorIterator<T, isConst>&
		CachedVectorIterator<T, isConst>::operator--()
	{
		do {
			mIndex--;
		}
		while (
			!mVector.isActive(mIndex)
			&& (mIndex < mVector.mElements.size())
		);

		return *this;
	}


	template<class T, bool isConst>
	CachedVectorIterator<T, isConst>
		CachedVectorIterator<T, isConst>::operator--(int)
	{
		CachedVectorIterator<T> ret(*this);
		operator--();
		return ret;
	}

}}

#endif		// CACHED_VECTOR_H
