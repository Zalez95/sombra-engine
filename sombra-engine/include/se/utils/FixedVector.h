#ifndef FIXED_VECTOR_H
#define FIXED_VECTOR_H

#include <cstddef>
#include <iterator>
#include <initializer_list>

namespace se::utils {

	/**
	 * Class FixedVector, a FixedVector is a container with a compile-time
	 * fixed size but which can vary the number of elements stored inside.
	 * Internally the vector stores the elements contiguously so they could be
	 * accessed with pointers and offsets. The FixedVector won't do any
	 * allocation at runtime.
	 *
	 * @tparam	N the maximum number of elements that can be stored in the
	 *			container
	 */
	template <typename T, std::size_t N>
	class FixedVector
	{
	public:		// Nested types
		using size_type					= std::size_t;
		using difference_type			= std::ptrdiff_t;
		using value_type				= T;
		using pointer					= T*;
		using reference					= T&;
		using iterator					= T*;
		using const_iterator			= const T*;
		using reverse_iterator			= std::reverse_iterator<iterator>;
		using const_reverse_iterator	= std::reverse_iterator<const_iterator>;

	private:	// Attributes
		/** The array of memory where the elements will be stored */
		std::byte mData[N * sizeof(T)];

		/** The number of Elements added to the FixedVector */
		size_type mNumElements;

	public:		// Functions
		/** Creates a new FixedVector */
		FixedVector() : mNumElements(0) {};
		FixedVector(const FixedVector& other);
		FixedVector(FixedVector&& other);

		/** Creates a new FixedVector
		 *
		 * @param	numElements the initial number of elements stored inside
		 *			the FixedVector. If it's larger than the maximum number of
		 *			elements the FixedVector can store, it will be set to 0 */
		FixedVector(size_type numElements) : mNumElements(0)
		{ resize(numElements); };

		/** Creates a new FixedVector
		 *
		 * @param	list the initial elements stored inside the FixedVector
		 * @note	it will be stored up to N elements */
		FixedVector(std::initializer_list<T> list);

		/** Creates a new FixedVector
		 *
		 * @param	first the first iterator of the range to copy
		 * @param	last the past-the-end iterator of the range to copy */
		template <typename InputIterator>
		FixedVector(InputIterator first, InputIterator last);

		/** Class destructor */
		~FixedVector();

		/** Assignment operator */
		FixedVector& operator=(const FixedVector& other);
		FixedVector& operator=(FixedVector&& other);

		/** Returns the Element i of the FixedVector
		 *
		 * @param	i the index of the Element
		 * @return	a reference to the Element */
		T& operator[](size_type i)
		{ return reinterpret_cast<T*>(mData)[i]; };

		/** Returns the Element i of the FixedVector
		 *
		 * @param	i the index of the Element
		 * @return	a const reference to the Element */
		const T& operator[](size_type i) const
		{ return reinterpret_cast<const T*>(mData)[i]; };

		/** Compares the given FixedVectors
		 *
		 * @param	lhs the first FixedVector to compare
		 * @param	rhs the second FixedVector to compare
		 * @return	true if both FixedVector are equal, false otherwise */
		template <typename U, std::size_t N2>
		friend bool operator==(
			const FixedVector<U, N2>& lhs, const FixedVector<U, N2>& rhs
		);

		/** Compares the given FixedVectors
		 *
		 * @param	lhs the first FixedVector to compare
		 * @param	rhs the second FixedVector to compare
		 * @return	true if both FixedVector are different, false otherwise */
		template <typename U, std::size_t N2>
		friend bool operator!=(
			const FixedVector<U, N2>& lhs, const FixedVector<U, N2>& rhs
		);

		/** @return	the initial iterator of the FixedVector */
		iterator begin()
		{ return reinterpret_cast<T*>(mData); };

		/** @return	the initial iterator of the FixedVector */
		const_iterator begin() const
		{ return reinterpret_cast<const T*>(mData); };

		/** @return	the final iterator of the FixedVector */
		iterator end()
		{ return reinterpret_cast<T*>(mData) + mNumElements; };

		/** @return	the final iterator of the FixedVector */
		const_iterator end() const
		{ return reinterpret_cast<const T*>(mData) + mNumElements; };

		/** @return	the initial reverse iterator of the FixedVector */
		reverse_iterator rbegin()
		{ return reinterpret_cast<T*>(mData) + mNumElements; };

		/** @return	the final reverse iterator of the FixedVector */
		const_reverse_iterator rbegin() const
		{ return reinterpret_cast<const T*>(mData) + mNumElements; };

		/** @return	the final reverse iterator of the FixedVector */
		reverse_iterator rend()
		{ return reinterpret_cast<T*>(mData); };

		/** @return	the initial reverse iterator of the FixedVector */
		const_reverse_iterator rend() const
		{ return reinterpret_cast<const T*>(mData); };

		/** @return	the number of Elements in the FixedVector */
		size_type size() const { return mNumElements; };

		/** @return	the maximum number of Elements that can be stored in the
		 *			FixedVector */
		size_type capacity() const { return N; };

		/** @return	true if the FixedVector has no Elements inside, false
		 *			otherwise */
		bool empty() const { return (mNumElements == 0); };

		/** @return	a reference to the first element in the FixedVector
		 * @note	if the FixedVector is empty, it causes undefined behavior */
		T& front()
		{ return reinterpret_cast<T*>(mData)[0]; };

		/** @return	a reference to the first element in the FixedVector
		 * @note	if the FixedVector is empty, it causes undefined behavior */
		const T& front() const
		{ return reinterpret_cast<const T*>(mData)[0]; };

		/** @return	a reference to the last element in the FixedVector
		 * @note	if the FixedVector is empty, it causes undefined behavior */
		T& back()
		{ return reinterpret_cast<T*>(mData)[mNumElements - 1]; };

		/** @return	a reference to the last element in the FixedVector
		 * @note	if the FixedVector is empty, it causes undefined behavior */
		const T& back() const
		{ return reinterpret_cast<const T*>(mData)[mNumElements - 1]; };

		/** @return	a pointer to the first element stored in the FixedVector */
		T* data()
		{ return reinterpret_cast<T*>(mData); };

		/** @return	a pointer to the first element stored in the FixedVector */
		const T* data() const
		{ return reinterpret_cast<const T*>(mData); };

		/** Removes all the elements in the FixedVector */
		void clear();

		/** Changes the number of elements stored in the FixedVector
		 *
		 * @param	numElements the new number of elements stored inside the
		 *			FixedVector
		 * @note	the new number of elements will be set to @tparam N if
		 *			numElements is larger */
		void resize(size_type numElements);

		/** Adds a new Element at the back of the FixedVector
		 *
		 * @param	element the Element to push at the back of the FixedVector
		 * @note	if the FixedVector is full, it causes undefined behavior */
		void push_back(T element);

		/** Adds a new Element at the back of the FixedVector
		 *
		 * @param	args the arguments needed for calling the constructor of
		 *			the new Element
		 * @return	a reference to the new element
		 * @note	if the FixedVector is full, it causes undefined behavior */
		template <typename... Args>
		T& emplace_back(Args&&... args);

		/** Removes the last Element of the FixedVector
		 * @note	if the FixedVector is empty, it causes undefined behavior */
		void pop_back();

		/** Inserts the given element into the FixedVector at the position
		 * located at the given iterator
		 *
		 * @param	it an iterator to the position to insert the element
		 * @param	value the element to insert
		 * @note	the next stored elements will be displaced one position
		 *			right inside the FixedVector */
		iterator insert(const_iterator it, const T& value);

		/** Inserts the given element into the FixedVector at the position
		 * located at the given iterator
		 *
		 * @param	it an iterator to the position to insert the element
		 * @param	args the arguments needed for calling the constructor of
		 *			the new Element
		 * @note	the next stored elements will be displaced one position
		 *			right inside the FixedVector */
		template <typename... Args>
		iterator emplace(const_iterator it, Args&&... args);

		/** Removes the element located at the given iterator from the
		 * FixedVector
		 *
		 * @param	it an iterator to the Element
		 * @note	the next stored elements will be displaced one position
		 *			left inside the FixedVector */
		iterator erase(const_iterator it);
	};

}

#include "FixedVector.hpp"

#endif		// FIXED_VECTOR_H
