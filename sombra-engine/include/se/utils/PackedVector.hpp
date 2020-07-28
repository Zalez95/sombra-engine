#ifndef PACKED_VECTOR_HPP
#define PACKED_VECTOR_HPP

namespace se::utils {

	template <typename T>
	bool operator==(const PackedVector<T>& cv1, const PackedVector<T>& cv2)
	{
		return (cv1.mElements == cv2.mElements)
			&& (cv1.mFreeIndices == cv2.mFreeIndices)
			&& (cv1.mNumElements == cv2.mNumElements);
	}


	template <typename T>
	bool operator!=(const PackedVector<T>& cv1, const PackedVector<T>& cv2)
	{
		return !(cv1 == cv2);
	}


	template <typename T>
	void PackedVector<T>::reserve(std::size_t n)
	{
		mElements.reserve(n);
		mFreeIndices.reserve(n);
	}


	template <typename T>
	void PackedVector<T>::clear()
	{
		mElements.clear();
		mFreeIndices.clear();
		mNumElements = 0;
	}


	template <typename T>
	template <typename... Args>
	typename PackedVector<T>::iterator PackedVector<T>::emplace(Args&&... args)
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

			mElements[index].~T();
			new (&mElements[index]) T(std::forward<Args>(args)...);
		}
		mNumElements++;

		return iterator(this, index);
	}


	template <typename T>
	typename PackedVector<T>::iterator PackedVector<T>::erase(const_iterator it)
	{
		iterator ret = it;
		++ret;

		size_type index = it.getIndex();
		if (isActive(index)) {
			mFreeIndices.insert(index);
			mNumElements--;
		}

		return ret;
	}


	template <typename T>
	bool PackedVector<T>::isActive(size_type i) const
	{
		return (i < mElements.size()) && (mFreeIndices.find(i) == mFreeIndices.end());
	}


	template <typename T>
	template <typename U>
	void PackedVector<T>::replicate(const PackedVector<U>& other)
	{
		mElements = std::vector<T>(other.mElements.size());
		mFreeIndices = other.mFreeIndices;
		mNumElements = other.mNumElements;
	}


	template <typename T>
	template <bool isConst>
	PackedVector<T>::PVIterator<isConst>::PVIterator(VectorType* vector) :
		mVector(vector), mIndex(0)
	{
		if (!mVector->isActive(mIndex) && (mVector->mNumElements > 0)) {
			operator++();
		}
	}


	template <typename T>
	template <bool isConst>
	PackedVector<T>::PVIterator<isConst>::operator
		PackedVector<T>::PVIterator<!isConst>() const
	{
		PVIterator<!isConst> ret(nullptr, mIndex);

		if constexpr (isConst) {
			ret.mVector = const_cast<PackedVector*>(mVector);
		}
		else {
			ret.mVector = mVector;
		}

		return ret;
	}


	template <typename T>
	template <bool isConst>
	PackedVector<T>::PVIterator<isConst>&
		PackedVector<T>::PVIterator<isConst>::setIndex(size_type index)
	{
		mIndex = index;
		return *this;
	}


	template <typename T>
	template <bool isConst>
	PackedVector<T>::PVIterator<isConst>&
		PackedVector<T>::PVIterator<isConst>::operator++()
	{
		do {
			++mIndex;
		}
		while (!mVector->isActive(mIndex) && (mIndex < mVector->mElements.size()));

		return *this;
	}


	template <typename T>
	template <bool isConst>
	PackedVector<T>::PVIterator<isConst>
		PackedVector<T>::PVIterator<isConst>::operator++(int)
	{
		PVIterator ret(*this);
		operator++();
		return ret;
	}


	template <typename T>
	template <bool isConst>
	PackedVector<T>::PVIterator<isConst>&
		PackedVector<T>::PVIterator<isConst>::operator--()
	{
		do {
			--mIndex;
		}
		while (!mVector->isActive(mIndex) && (mIndex < mVector->mElements.size()));

		return *this;
	}


	template <typename T>
	template <bool isConst>
	PackedVector<T>::PVIterator<isConst>
		PackedVector<T>::PVIterator<isConst>::operator--(int)
	{
		PVIterator ret(*this);
		operator--();
		return ret;
	}

}

#endif		// PACKED_VECTOR_HPP
