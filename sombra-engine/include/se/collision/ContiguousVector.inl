#ifndef CONTIGUOUS_VECTOR_INL
#define CONTIGUOUS_VECTOR_INL

namespace se::collision {

	template <typename T>
	bool ContiguousVector<T>::operator==(const ContiguousVector& other) const
	{
		return (mElements == other.mElements)
			&& (mNumElements == other.mNumElements)
			&& (mFreeIndices == other.mFreeIndices);
	}


	template <typename T>
	template <typename... Args>
	typename ContiguousVector<T>::size_type ContiguousVector<T>::create(Args&&... args)
	{
		size_type index;
		if (mFreeIndices.empty()) {
			mElements.emplace_back(std::forward<Args>(args)...);
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


	template <typename T>
	void ContiguousVector<T>::release(size_type i)
	{
		if (isActive(i)) {
			mFreeIndices.insert(i);
			mElements[i] = T();
			mNumElements--;
		}
	}


	template <typename T>
	bool ContiguousVector<T>::isActive(size_type i) const
	{
		return (i < mElements.size()) && (mFreeIndices.find(i) == mFreeIndices.end());
	}


	template <typename T>
	template <bool isConst>
	ContiguousVector<T>::CVIterator<isConst>::CVIterator(VectorType* vector) :
		mVector(vector), mIndex(0)
	{
		if (!mVector->isActive(mIndex) && (mVector->mNumElements > 0)) {
			operator++();
		}
	}


	template <typename T>
	template <bool isConst>
	bool ContiguousVector<T>::CVIterator<isConst>::operator==(const CVIterator& other) const
	{
		return (mVector == other.mVector) && (mIndex == other.mIndex);
	}


	template <typename T>
	template <bool isConst>
	bool ContiguousVector<T>::CVIterator<isConst>::operator!=(const CVIterator& other) const
	{
		return !operator==(other);
	}


	template <typename T>
	template <bool isConst>
	ContiguousVector<T>::CVIterator<isConst>& ContiguousVector<T>::CVIterator<isConst>::operator++()
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


	template <typename T>
	template <bool isConst>
	ContiguousVector<T>::CVIterator<isConst> ContiguousVector<T>::CVIterator<isConst>::operator++(int)
	{
		CVIterator ret(*this);
		operator++();
		return ret;
	}


	template <typename T>
	template <bool isConst>
	ContiguousVector<T>::CVIterator<isConst>& ContiguousVector<T>::CVIterator<isConst>::operator--()
	{
		do {
			mIndex--;
		}
		while (!mVector->isActive(mIndex) && (mIndex < mVector->mElements.size()));

		return *this;
	}


	template <typename T>
	template <bool isConst>
	ContiguousVector<T>::CVIterator<isConst>
		ContiguousVector<T>::CVIterator<isConst>::operator--(int)
	{
		CVIterator ret(*this);
		operator--();
		return ret;
	}

}

#endif		// CONTIGUOUS_VECTOR_INL
