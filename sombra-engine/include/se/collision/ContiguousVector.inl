#ifndef CONTIGUOUS_VECTOR_INL
#define CONTIGUOUS_VECTOR_INL

namespace se::collision {

	template <typename T>
	bool operator==(const ContiguousVector<T>& cv1, const ContiguousVector<T>& cv2)
	{
		return (cv1.mElements == cv2.mElements)
			&& (cv1.mNumElements == cv2.mNumElements)
			&& (cv1.mFreeIndices == cv2.mFreeIndices);
	}


	template <typename T>
	template <typename... Args>
	typename ContiguousVector<T>::iterator ContiguousVector<T>::emplace(Args&&... args)
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

		return iterator(this, index);
	}


	template <typename T>
	void ContiguousVector<T>::erase(const_iterator it)
	{
		size_type index = it.getIndex();
		if (isActive(index)) {
			mFreeIndices.insert(index);
			mElements[index] = T();
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
	ContiguousVector<T>::CVIterator<isConst>::operator
		ContiguousVector<T>::CVIterator<!isConst>() const
	{
		CVIterator<!isConst> ret(nullptr, mIndex);

		if constexpr (isConst) {
			ret.mVector = const_cast<ContiguousVector*>(mVector);
		}
		else {
			ret.mVector = mVector;
		}

		return ret;
	}


	template <typename T>
	template <bool isConst>
	ContiguousVector<T>::CVIterator<isConst>&
		ContiguousVector<T>::CVIterator<isConst>::setIndex(size_type index)
	{
		mIndex = index;
		return *this;
	}


	template <typename T>
	template <bool isConst>
	ContiguousVector<T>::CVIterator<isConst>&
		ContiguousVector<T>::CVIterator<isConst>::operator++()
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
	ContiguousVector<T>::CVIterator<isConst>
		ContiguousVector<T>::CVIterator<isConst>::operator++(int)
	{
		CVIterator ret(*this);
		operator++();
		return ret;
	}


	template <typename T>
	template <bool isConst>
	ContiguousVector<T>::CVIterator<isConst>&
		ContiguousVector<T>::CVIterator<isConst>::operator--()
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
