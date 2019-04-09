#ifndef FIXED_VECTOR_INL
#define FIXED_VECTOR_INL

namespace se::utils {

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

		return iterator(this, mNumElements-1);
	}


	template <typename T, std::size_t N>
	template <typename... Args>
	typename FixedVector<T, N>::iterator FixedVector<T, N>::emplace_back(Args&&... args)
	{
		mElements[mNumElements] = T(std::forward<Args>(args)...);
		mNumElements++;

		return iterator(this, mNumElements-1);
	}


	template <typename T, std::size_t N>
	void FixedVector<T, N>::pop_back()
	{
		mNumElements--;
	}


	template <typename T, std::size_t N>
	typename FixedVector<T, N>::iterator FixedVector<T, N>::erase(const_iterator it)
	{
		for (size_type i = it.mIndex + 1; i < mNumElements; ++i) {
			mElements[i - 1] = mElements[i];
		}
		mNumElements--;
	}


	template <typename T, std::size_t N>
	template <bool isConst>
	FixedVector<T, N>::FVIterator<isConst>::operator
		FixedVector<T, N>::FVIterator<!isConst>() const
	{
		FVIterator<!isConst> ret(nullptr, mIndex);

		if constexpr (isConst) {
			ret.mVector = const_cast<FixedVector*>(mVector);
		}
		else {
			ret.mVector = mVector;
		}

		return ret;
	}


	template <typename T, std::size_t N>
	template <bool isConst>
	FixedVector<T, N>::FVIterator<isConst>&
		FixedVector<T, N>::FVIterator<isConst>::operator++()
	{
		mIndex++;
		return *this;
	}


	template <typename T, std::size_t N>
	template <bool isConst>
	FixedVector<T, N>::FVIterator<isConst>
		FixedVector<T, N>::FVIterator<isConst>::operator++(int)
	{
		FVIterator ret(*this);
		operator++();
		return ret;
	}


	template <typename T, std::size_t N>
	template <bool isConst>
	FixedVector<T, N>::FVIterator<isConst>&
		FixedVector<T, N>::FVIterator<isConst>::operator--()
	{
		mIndex--;
		return *this;
	}


	template <typename T, std::size_t N>
	template <bool isConst>
	FixedVector<T, N>::FVIterator<isConst>
		FixedVector<T, N>::FVIterator<isConst>::operator--(int)
	{
		FVIterator ret(*this);
		operator--();
		return ret;
	}

}

#endif		// FIXED_VECTOR_INL
