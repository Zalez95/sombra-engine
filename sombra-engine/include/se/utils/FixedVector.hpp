#ifndef FIXED_VECTOR_HPP
#define FIXED_VECTOR_HPP

namespace se::utils {

	template <typename T, std::size_t N>
	FixedVector<T, N>::FixedVector(std::initializer_list<T> list) :
		mNumElements( (N < list.size())? N : list.size() )
	{
		size_type i = 0;
		for (const T& member : list) {
			if (i < mNumElements) {
				mElements[i] = member;
			}
			++i;
		}
	}


	template <typename T, std::size_t N>
	template <typename InputIterator>
	FixedVector<T, N>::FixedVector(InputIterator first, InputIterator last) :
		mNumElements(0)
	{
		for (auto it = first; it != last; ++it) {
			push_back(*it);
		}
	}


	template <typename T, std::size_t N>
	bool operator==(const FixedVector<T, N>& fv1, const FixedVector<T, N>& fv2)
	{
		bool equal = true;

		if (fv1.mNumElements == fv2.mNumElements) {
			for (auto i = 0; i < fv1.mNumElements; ++i) {
				if (fv1.mElements[i] != fv2.mElements[i]) {
					equal = false;
					break;
				}
			}
		}
		else {
			equal = false;
		}

		return equal;
	}


	template <typename T, std::size_t N>
	bool operator!=(const FixedVector<T, N>& fv1, const FixedVector<T, N>& fv2)
	{
		return !(fv1 == fv2);
	}


	template <typename T, std::size_t N>
	void FixedVector<T, N>::clear()
	{
		resize(0);
	}


	template <typename T, std::size_t N>
	void FixedVector<T, N>::resize(size_type numElements)
	{
		if (numElements <= N) {
			mNumElements = numElements;
		}
	}


	template <typename T, std::size_t N>
	typename FixedVector<T, N>::iterator FixedVector<T, N>::push_back(T element)
	{
		mElements[mNumElements] = element;
		mNumElements++;

		return &mElements[mNumElements - 1];
	}


	template <typename T, std::size_t N>
	template <typename... Args>
	typename FixedVector<T, N>::iterator FixedVector<T, N>::emplace_back(Args&&... args)
	{
		return push_back( T(std::forward<Args>(args)...) );
	}


	template <typename T, std::size_t N>
	void FixedVector<T, N>::pop_back()
	{
		mNumElements--;
	}


	template <typename T, std::size_t N>
	typename FixedVector<T, N>::iterator FixedVector<T, N>::insert(const_iterator it, const T& value)
	{
		iterator itCopy = const_cast<iterator>(it);

		mNumElements++;
		for (iterator it2 = end() - 1; it2 != itCopy; --it2) {
			*it2 = *(it2 - 1);
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
				*(it2 - 1) = *it2;
			}
			mNumElements--;
		}

		return itCopy;
	}

}

#endif		// FIXED_VECTOR_HPP
