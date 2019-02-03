#ifndef CONTIGUOUS_VECTOR_H
#define CONTIGUOUS_VECTOR_H

#include <set>
#include <vector>

namespace se::collision {

	/**
	 * Class ContiguousVector, it works as an usual vector but it also caches
	 * the released elements instead of erasing them for preventing the old
	 * indices pointing to the vector from being invalidated.
	 *
	 * @note	it doesn't prevent from pointer invalidations due to the
	 *			increment of the vector size with new allocations, also the
	 *			released elements will be reused in the following allocations
	 */
	template <typename T>
	class ContiguousVector
	{
	public:		// Nested types
		/** Class CVIterator, it's the class used to iterate through the
		 * elements of a ContiguousVector */
		template <bool isConst>
		class CVIterator
		{
		public:		// Nested types
			template <bool isConst2>
			friend class CVIterator;

			using VectorType = std::conditional_t<isConst,
				const ContiguousVector, ContiguousVector
			>;

			using size_type			= typename VectorType::size_type;
			using difference_type	= long;
			using value_type		= std::conditional_t<isConst, const T, T>;
			using pointer			= value_type*;
			using reference			= value_type&;
			using iterator_category	= std::bidirectional_iterator_tag;

		private:	// Attributes
			/** A pointer to the vector to iterate */
			VectorType* mVector;

			/** The current position in the vector */
			size_type mIndex;

		public:		// Functions
			/** Creates a new CVIterator located at the initial valid position
			 * of the given ContiguousVector (begin)
			 *
			 * @param	vector a pointer to the vector to iterate */
			CVIterator(VectorType* vector);

			/** Creates a new CVIterator
			 *
			 * @param	vector a pointer to the vector to iterate
			 * @param	index the inital index of the iterator */
			CVIterator(VectorType* vector, size_type index) :
				mVector(vector), mIndex(index) {};

			/** Implicit conversion operator between const CVIterator and non
			 * const CVIterator
			 *
			 * @return	the new CVIterator with a different template const
			 *			type */
			operator CVIterator<!isConst>() const;

			/** @return	the index of the Element that the iterator is pointing
			 *			to */
			size_type getIndex() const { return mIndex; };

			/** @return	the index of the Element that the iterator is pointing
			 *			to */
			CVIterator& setIndex(size_type index);

			/** @return	a reference to the current Element that the iterator is
			 *			pointing to */
			reference operator*() const { return (*mVector)[mIndex]; };

			/** @return	a pointer to the current Element that the iterator is
			 *			pointing to */
			pointer operator->() { return &(*mVector)[mIndex]; };

			/** Compares the given CVIterators
			 *
			 * @param	it1 the first iterator to compare
			 * @param	it2 the second iterator to compare
			 * @return	true if both iterators are equal, false otherwise */
			friend bool operator==(const CVIterator& it1, const CVIterator& it2)
			{ return it1.mVector == it2.mVector && it1.mIndex == it2.mIndex; };

			/** Compares the given CVIterators
			 *
			 * @param	it1 the first iterator to compare
			 * @param	it2 the second iterator to compare
			 * @return	true if both iterators are different, false otherwise */
			friend bool operator!=(const CVIterator& it1, const CVIterator& it2)
			{ return !(it1 == it2); };

			/** Preincrement operator
			 *
			 * @return	a reference to the current iterator after its
			 *			incrementation */
			CVIterator& operator++();

			/** Postincrement operator
			 *
			 * @return	a copy of the current iterator with the previous value
			 *			to the incrementation */
			CVIterator operator++(int);

			/** Predecrement operator
			 *
			 * @return	a reference to the current iterator after its
			 *			decrementation */
			CVIterator& operator--();

			/** Postdecrement operator
			 *
			 * @return	a copy of the current iterator with the previous value
			 *			to the decrementation */
			CVIterator operator--(int);
		};

		using size_type			= std::size_t;
		using value_type		= T;
		using pointer			= T*;
		using reference			= T&;
		using iterator			= CVIterator<false>;
		using const_iterator	= CVIterator<true>;

	private:	// Attributes
		/** The raw data of the ContiguousVector */
		std::vector<T> mElements;

		/** The number of non free Elements of the ContiguousVector */
		size_type mNumElements;

		/** The indices to the freed Elements of the ContiguousVector */
		std::set<size_type> mFreeIndices;

	public:		// Functions
		/** Creates a new ContiguousVector */
		ContiguousVector() : mNumElements(0) {};

		/** Returns the Element i of the ContiguousVector
		 *
		 * @param	i the index of the Element
		 * @return	a reference to the Element */
		T& operator[](size_type i) { return mElements[i]; };

		/** Returns the Element i of the ContiguousVector
		 *
		 * @param	i the index of the Element
		 * @return	a const reference to the Element */
		const T& operator[](size_type i) const { return mElements[i]; };

		/** Compares the given ContiguousVectors
		 *
		 * @param	cv1 the first ContiguousVector to compare
		 * @param	cv2 the second ContiguousVector to compare
		 * @return	true if both ContiguousVector are equal, false otherwise */
		template <typename U>
		friend bool operator==(
			const ContiguousVector& cv1, const ContiguousVector& cv2
		);

		/** Compares the given ContiguousVectors
		 *
		 * @param	cv1 the first ContiguousVector to compare
		 * @param	cv2 the second ContiguousVector to compare
		 * @return	true if both ContiguousVector are different, false
		 *			otherwise */
		template <typename U>
		friend bool operator!=(
			const ContiguousVector& cv1, const ContiguousVector& cv2
		);

		/** @return	the initial iterator of the ContiguousVector */
		iterator begin() { return iterator(this); };

		/** @return	the initial iterator of the ContiguousVector */
		const_iterator begin() const { return const_iterator(this); };

		/** @return	the final iterator of the ContiguousVector */
		iterator end() { return iterator(this, mElements.size()); };

		/** @return	the final iterator of the ContiguousVector */
		const_iterator end() const
		{ return const_iterator(this, mElements.size()); };

		/** @return	the number of Elements in the ContiguousVector */
		size_type size() const { return mNumElements; };

		/** @return	true if the ContiguousVector has no Elements inside, false
		 *			otherwise */
		bool empty() const { return (mNumElements == 0); };

		/** Creates a new Element in the vector or reuses an already released
		 * one
		 *
		 * @param	args the arguments needed for calling the constructor of
		 *			the new Element
		 * @return	an iterator to the element */
		template <typename... Args>
		iterator emplace(Args&&... args);

		/** Removes the element located at the given iterator from the
		 * ContiguousVector
		 *
		 * @param	it an iterator to the Element
		 * @note	the element will be released instead of deleted so the
		 *			indices to the other elements won't need to be updated */
		void erase(const_iterator it);

		/** Checks if the Element given located at the given index is valid and
		 * active
		 *
		 * @param	i the index of the Element
		 * @return	true if is valid and active false otherwise */
		bool isActive(size_type i) const;
	};

}

#include "ContiguousVector.inl"

#endif		// CONTIGUOUS_VECTOR_H
