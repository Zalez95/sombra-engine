#ifndef FIXED_VECTOR_HPP
#define FIXED_VECTOR_HPP

#include <memory>
#include <algorithm>

namespace se::utils {

	template <typename T, std::size_t N>
	FixedVector<T, N>::FixedVector(const FixedVector& other) : mNumElements(other.mNumElements)
	{
		std::uninitialized_copy(other.begin(), other.end(), reinterpret_cast<T*>(mData));
	}


	template <typename T, std::size_t N>
	FixedVector<T, N>::FixedVector(FixedVector&& other) : mNumElements(other.mNumElements)
	{
		std::uninitialized_move(other.begin(), other.end(), reinterpret_cast<T*>(mData));
		other.clear();
	}


	template <typename T, std::size_t N>
	FixedVector<T, N>::FixedVector(std::initializer_list<T> list) : mNumElements(0)
	{
		size_type i = 0;
		for (const T& member : list) {
			if (i < N) {
				push_back(member);
			}
			++i;
		}
	}


	template <typename T, std::size_t N>
	template <typename InputIterator>
	FixedVector<T, N>::FixedVector(InputIterator first, InputIterator last) : mNumElements(0)
	{
		std::copy(first, last, std::back_inserter(*this));
	}


	template <typename T, std::size_t N>
	FixedVector<T, N>::~FixedVector()
	{
		clear();
	}


	template <typename T, std::size_t N>
	FixedVector<T, N>& FixedVector<T, N>::operator=(const FixedVector& other)
	{
		clear();
		mNumElements = other.mNumElements;
		std::uninitialized_copy(other.begin(), other.end(), reinterpret_cast<T*>(mData));

		return *this;
	}


	template <typename T, std::size_t N>
	FixedVector<T, N>& FixedVector<T, N>::operator=(FixedVector&& other)
	{
		clear();
		mNumElements = other.mNumElements;
		std::uninitialized_move(other.begin(), other.end(), reinterpret_cast<T*>(mData));
		other.clear();

		return *this;
	}


	template <typename T, std::size_t N>
	bool operator==(const FixedVector<T, N>& lhs, const FixedVector<T, N>& rhs)
	{
		return (lhs.mNumElements == rhs.mNumElements)
			&& std::equal(lhs.begin(), lhs.end(), rhs.end());
	}


	template <typename T, std::size_t N>
	bool operator!=(const FixedVector<T, N>& lhs, const FixedVector<T, N>& rhs)
	{
		return !(lhs == rhs);
	}


	template <typename T, std::size_t N>
	void FixedVector<T, N>::clear()
	{
		while (!empty()) {
			pop_back();
		}
	}


	template <typename T, std::size_t N>
	void FixedVector<T, N>::resize(size_type numElements)
	{
		if (numElements > N) {
			numElements = N;
		}

		while (numElements < mNumElements) {
			pop_back();
		}

		while (numElements > mNumElements) {
			emplace_back();
		}
	}


	template <typename T, std::size_t N>
	void FixedVector<T, N>::push_back(T element)
	{
		emplace_back(element);
	}


	template <typename T, std::size_t N>
	template <typename... Args>
	T& FixedVector<T, N>::emplace_back(Args&&... args)
	{
		new (mData + mNumElements * sizeof(T)) T(std::forward<Args>(args)...);
		mNumElements++;

		return reinterpret_cast<T*>(mData)[mNumElements - 1];
	}


	template <typename T, std::size_t N>
	void FixedVector<T, N>::pop_back()
	{
		back().~T();
		mNumElements--;
	}


	template <typename T, std::size_t N>
	typename FixedVector<T, N>::iterator FixedVector<T, N>::insert(const_iterator it, const T& value)
	{
		iterator itCopy = const_cast<iterator>(it);

		emplace_back( std::move(back()) );
		for (iterator it2 = end() - 2; it2 != itCopy; --it2) {
			*it2 = std::move(*(it2 - 1));
		}
		*itCopy = value;

		return itCopy;
	}


	template <typename T, std::size_t N>
	template <typename... Args>
	typename FixedVector<T, N>::iterator FixedVector<T, N>::emplace(const_iterator it, Args&&... args)
	{
		return insert(it, T(std::forward<Args>(args)...));
	}


	template <typename T, std::size_t N>
	typename FixedVector<T, N>::iterator FixedVector<T, N>::erase(const_iterator it)
	{
		iterator itCopy = const_cast<iterator>(it);

		if (itCopy != end()) {
			for (iterator it2 = itCopy + 1; it2 != end(); ++it2) {
				*(it2 - 1) = std::move(*it2);
			}
			pop_back();
		}

		return itCopy;
	}

}

#endif		// FIXED_VECTOR_HPP
