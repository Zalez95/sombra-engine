#ifndef FIXED_VECTOR_H
#define FIXED_VECTOR_H

namespace se::utils {

	/**
	 * Class FixedVector, a FixedVector is a container with a compile-time
	 * fixed size but which can vary the number of elements stored inside.
	 * Internally the vector stores the elements contiguously so they could be
	 * accessed with pointers and offsets. The FixedVector won't do any
	 * allocation at runtime.
	 */
	template <typename T, std::size_t N>
	class FixedVector
	{
	public:		// Nested types
		/** Class FVIterator, it's the class used to iterate through the
		 * elements of a FixedVector */
		template <bool isConst>
		class FVIterator
		{
		public:		// Nested types
			template <bool isConst2>
			friend class FVIterator;

			using VectorType = std::conditional_t<isConst,
				const FixedVector, FixedVector
			>;

			using size_type			= typename VectorType::size_type;
			using difference_type	= long;
			using value_type		= std::conditional_t<isConst, const T, T>;
			using pointer			= value_type*;
			using reference			= value_type&;
			using iterator_category	= std::bidirectional_iterator_tag;

		private:	// Attributes
			/** A reference to the vector to iterate */
			VectorType* mVector;

			/** The current position in the vector */
			size_type mIndex;

		public:		// Functions
			/** Creates a new FVIterator located at the initial valid position
			 * of the given FixedVector (begin)
			 *
			 * @param	vector a reference to the vector to iterate */
			FVIterator(VectorType* vector) : mVector(vector), mIndex(0) {};

			/** Creates a new FVIterator
			 *
			 * @param	vector a reference to the vector to iterate
			 * @param	index the inital index of the iterator */
			FVIterator(VectorType* vector, size_type index) :
				mVector(vector), mIndex(index) {};

			/** Implicit conversion operator between const FVIterator and non
			 * const FVIterator
			 *
			 * @return	the new FVIterator with a different template const
			 *			type */
			operator FVIterator<!isConst>() const;

			/** @return	a reference to the current Element that the iterator is
			 *			pointing to */
			reference operator*() const { return (*mVector)[mIndex]; };

			/** @return	a pointer to the current Element that the iterator is
			 *			pointing to */
			pointer operator->() { return &(*mVector)[mIndex]; };

			/** Compares the given FVIterators
			 *
			 * @param	it1 the first iterator to compare
			 * @param	it2 the second iterator to compare
			 * @return	true if both iterators are equal, false otherwise */
			friend bool operator==(const FVIterator& it1, const FVIterator& it2)
			{ return it1.mVector == it2.mVector && it1.mIndex == it2.mIndex; };

			/** Compares the given FVIterators
			 *
			 * @param	it1 the first iterator to compare
			 * @param	it2 the second iterator to compare
			 * @return	true if both iterators are different, false otherwise */
			friend bool operator!=(const FVIterator& it1, const FVIterator& it2)
			{ return !(it1 == it2); };

			/** Preincrement operator
			 *
			 * @return	a reference to the current iterator after its
			 *			incrementation */
			FVIterator& operator++();

			/** Postincrement operator
			 *
			 * @return	a copy of the current iterator with the previous value
			 *			to the incrementation */
			FVIterator operator++(int);

			/** Predecrement operator
			 *
			 * @return	a reference to the current iterator after its
			 *			decrementation */
			FVIterator& operator--();

			/** Postdecrement operator
			 *
			 * @return	a copy of the current iterator with the previous value
			 *			to the decrementation */
			FVIterator operator--(int);
		};

		using size_type			= std::size_t;
		using value_type		= T;
		using pointer			= T*;
		using reference			= T&;
		using iterator			= FVIterator<false>;
		using const_iterator	= FVIterator<true>;

	private:	// Attributes
		/** The array where the elements will be stored */
		T mElements[N];

		/** The number of Elements added to the FixedVector */
		size_type mNumElements;

	public:		// Functions
		/** Creates a new FixedVector
		 *
		 * @param	numElements the initial number of elements stored inside
		 *			the FixedVector. If it's larger than the maximum number of
		 *			elements the FixedVector can store, it will be set to 0 */
		FixedVector(size_type numElements = 0) : mNumElements(0)
		{ resize(numElements); };

		/** Returns the Element i of the FixedVector
		 *
		 * @param	i the index of the Element
		 * @return	a reference to the Element */
		T& operator[](size_type i) { return mElements[i]; };

		/** Returns the Element i of the FixedVector
		 *
		 * @param	i the index of the Element
		 * @return	a const reference to the Element */
		const T& operator[](size_type i) const { return mElements[i]; };

		/** Compares the given FixedVectors
		 *
		 * @param	fv1 the first FixedVector to compare
		 * @param	fv2 the second FixedVector to compare
		 * @return	true if both FixedVector are equal, false otherwise */
		template <typename U, std::size_t N2>
		friend bool operator==(
			const FixedVector<U, N2>& fv1, const FixedVector<U, N2>& fv2
		);

		/** Compares the given FixedVectors
		 *
		 * @param	fv1 the first FixedVector to compare
		 * @param	fv2 the second FixedVector to compare
		 * @return	true if both FixedVector are different, false otherwise */
		template <typename U, std::size_t N2>
		friend bool operator!=(
			const FixedVector<U, N2>& fv1, const FixedVector<U, N2>& fv2
		);

		/** @return	the initial iterator of the FixedVector */
		iterator begin() { return iterator(this); };

		/** @return	the initial iterator of the FixedVector */
		const_iterator begin() const { return const_iterator(this); };

		/** @return	the final iterator of the FixedVector */
		iterator end() { return iterator(this, mNumElements); };

		/** @return	the final iterator of the FixedVector */
		const_iterator end() const
		{ return const_iterator(this, mNumElements); };

		/** @return	the number of Elements in the FixedVector */
		size_type size() const { return mNumElements; };

		/** @return	the maximum number of Elements that can be stored in the
		 *			FixedVector */
		size_type capacity() const { return N; };

		/** @return	true if the FixedVector has no Elements inside, false
		 *			otherwise */
		bool empty() const { return (mNumElements == 0); };

		/** @return	true if the FixedVector has reached its maximum number of
		 *			elements inside */
		bool full() const { return (mNumElements == N); };

		/** @return	a reference to the first element in the FixedVector
		 * @note	if the FixedVector is empty, it causes undefined behavior */
		T& front() { return mElements[0]; };

		/** @return	a reference to the first element in the FixedVector
		 * @note	if the FixedVector is empty, it causes undefined behavior */
		const T& front() const { return mElements[0]; };

		/** @return	a reference to the last element in the FixedVector
		 * @note	if the FixedVector is empty, it causes undefined behavior */
		T& back() { return mElements[mNumElements - 1]; };

		/** @return	a reference to the last element in the FixedVector
		 * @note	if the FixedVector is empty, it causes undefined behavior */
		const T& back() const { return mElements[mNumElements - 1]; };

		/** @return	a pointer to the first element stored in the FixedVector */
		T* data() { return &mElements[0]; };

		/** @return	a pointer to the first element stored in the FixedVector */
		const T* data() const { return &mElements[0]; };

		/** Removes all the elements in the FixedVector */
		void clear();

		/** Changes the number of elements stored in the FixedVector
		 *
		 * @param	numElements the new number of elements stored inside the
		 *			FixedVector
		 * @note	if the new number of elements is larger than the maximum
		 *			number of elements the FixedVector can hold, this function
		 *			will have no effect */
		void resize(size_type numElements);

		/** Adds a new Element at the back of the FixedVector
		 *
		 * @param	element the Element to push at the back of the FixedVector
		 * @return	an iterator to the element
		 * @note	if the FixedVector is full, it causes undefined behavior */
		iterator push_back(T element);

		/** Adds a new Element at the back of the FixedVector
		 *
		 * @param	args the arguments needed for calling the constructor of
		 *			the new Element
		 * @return	an iterator to the element
		 * @note	if the FixedVector is full, it causes undefined behavior */
		template <typename... Args>
		iterator emplace_back(Args&&... args);

		/** Removes the last Element of the FixedVector
		 * @note	if the FixedVector is empty, it causes undefined behavior */
		void pop_back();

		/** Removes the element located at the given iterator from the
		 * FixedVector
		 *
		 * @param	it an iterator to the Element
		 * @note	the next stored elements will be displaced one position
		 *			left inside the FixedVector */
		iterator erase(const_iterator it);
	};

}

#include "FixedVector.inl"

#endif		// FIXED_VECTOR_H
