#ifndef PACKED_VECTOR_HPP
#define PACKED_VECTOR_HPP

#include <algorithm>

namespace se::utils {

	template <typename T, typename A>
	PackedVector<T, A>::PackedVector(const PackedVector& other) :
		mElements(nullptr), mCapacity(0),
		mUsedElements(other.mUsedElements), mReleasedIndices(other.mReleasedIndices)
	{
		reserve(other.mCapacity);
		for (auto it = other.begin(); it != other.end(); ++it) {
			new (&mElements[it.getIndex()]) T(*it);
		}
	}


	template <typename T, typename A>
	PackedVector<T, A>::PackedVector(PackedVector&& other) :
		mElements(other.mElements), mCapacity(other.mCapacity),
		mUsedElements(other.mUsedElements), mReleasedIndices(std::move(other.mReleasedIndices))
	{
		other.mElements = nullptr;
		other.mCapacity = 0;
		other.mUsedElements = 0;
	}


	template <typename T, typename A>
	PackedVector<T, A>::PackedVector(
		const T* elements, std::size_t capacity, std::size_t size,
		const std::size_t* releasedIndices, std::size_t numReleasedIndices
	) : mElements(nullptr), mCapacity(0), mUsedElements(0)
	{
		reserve(capacity);
		mUsedElements = size + numReleasedIndices;
		std::copy(elements, elements + mUsedElements, mElements);
		std::copy(releasedIndices, releasedIndices + numReleasedIndices, std::back_inserter(mReleasedIndices));
	}


	template <typename T, typename A>
	PackedVector<T, A>::~PackedVector()
	{
		clear();
		mAllocator.deallocate(mElements, mCapacity);
	}


	template <typename T, typename A>
	PackedVector<T, A>& PackedVector<T, A>::operator=(const PackedVector& other)
	{
		clear();

		mUsedElements = other.mUsedElements;
		mReleasedIndices = other.mReleasedIndices;
		reserve(other.mCapacity);
		for (auto it = other.begin(); it != other.end(); ++it) {
			new (&mElements[it.getIndex()]) T(*it);
		}

		return *this;
	}


	template <typename T, typename A>
	PackedVector<T, A>& PackedVector<T, A>::operator=(PackedVector&& other)
	{
		clear();

		mElements = other.mElements;
		mCapacity = other.mCapacity;
		mUsedElements = other.mUsedElements;
		mReleasedIndices = std::move(other.mReleasedIndices);

		other.mElements = nullptr;
		other.mCapacity = 0;
		other.mUsedElements = 0;

		return *this;
	}


	template <typename T, typename A>
	bool operator==(const PackedVector<T, A>& cv1, const PackedVector<T, A>& cv2)
	{
		return (cv1.mElements == cv2.mElements)
			&& (cv1.mCapacity == cv2.mCapacity)
			&& (cv1.mUsedElements == cv2.mUsedElements)
			&& (cv1.mReleasedIndices == cv2.mReleasedIndices);
	}


	template <typename T, typename A>
	bool operator!=(const PackedVector<T, A>& cv1, const PackedVector<T, A>& cv2)
	{
		return !(cv1 == cv2);
	}


	template <typename T, typename A>
	void PackedVector<T, A>::reserve(std::size_t n)
	{
		if (n > mCapacity) {
			T* buffer = mAllocator.allocate(n);
			if (mCapacity > 0) {
				for (auto it = begin(); it != end(); ++it) {
					new (&buffer[it.getIndex()]) T(std::move(*it));
					(*it).~T();
				}
				mAllocator.deallocate(mElements, mCapacity);
			}

			mElements = buffer;
			mCapacity = n;
			mReleasedIndices.reserve(n);
		}
	}


	template <typename T, typename A>
	void PackedVector<T, A>::clear()
	{
		for (auto it = begin(); it != end();) {
			it = erase(it);
		}
	}


	template <typename T, typename A>
	template <typename... Args>
	typename PackedVector<T, A>::iterator PackedVector<T, A>::emplace(Args&&... args)
	{
		size_type index;
		if (mReleasedIndices.empty()) {
			if (mCapacity == 0) {
				reserve(1);
			}
			else if (mUsedElements + 1 > mCapacity) {
				reserve(2 * mCapacity);
			}
			index = mUsedElements++;
		}
		else {
			index = mReleasedIndices.back();
			mReleasedIndices.pop_back();
		}

		new (&mElements[index]) T(std::forward<Args>(args)...);
		return iterator(this, index);
	}


	template <typename T, typename A>
	typename PackedVector<T, A>::iterator PackedVector<T, A>::erase(const_iterator it)
	{
		iterator ret = it;
		++ret;

		size_type index = it.getIndex();
		if (isActive(index)) {
			mElements[index].~T();
			mReleasedIndices.push_back(index);
		}

		return ret;
	}


	template <typename T, typename A>
	bool PackedVector<T, A>::isActive(size_type i) const
	{
		return (i < mUsedElements)
			&& (std::find(mReleasedIndices.begin(), mReleasedIndices.end(), i) == mReleasedIndices.end());
	}


	template <typename T, typename A>
	template <typename U>
	void PackedVector<T, A>::replicate(const PackedVector<U>& other, const T& value)
	{
		clear();

		reserve(other.mCapacity);
		mUsedElements = other.mUsedElements;
		mReleasedIndices = other.mReleasedIndices;
		for (auto it = begin(); it != end(); ++it) {
			new (&(*it)) T(value);
		}
	}


	template <typename T, typename A>
	template <bool isConst>
	PackedVector<T, A>::PVIterator<isConst>::PVIterator(VectorType* vector) :
		mVector(vector), mIndex(0)
	{
		if (!mVector->isActive(mIndex) && (mVector->mUsedElements > 0)) {
			operator++();
		}
	}


	template <typename T, typename A>
	template <bool isConst>
	PackedVector<T, A>::PVIterator<isConst>::operator
		PackedVector<T, A>::PVIterator<!isConst>() const
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


	template <typename T, typename A>
	template <bool isConst>
	PackedVector<T, A>::PVIterator<isConst>&
		PackedVector<T, A>::PVIterator<isConst>::setIndex(size_type index)
	{
		mIndex = index;
		return *this;
	}


	template <typename T, typename A>
	template <bool isConst>
	PackedVector<T, A>::PVIterator<isConst>&
		PackedVector<T, A>::PVIterator<isConst>::operator++()
	{
		do {
			++mIndex;
		}
		while (!mVector->isActive(mIndex) && (mIndex < mVector->mUsedElements));

		return *this;
	}


	template <typename T, typename A>
	template <bool isConst>
	PackedVector<T, A>::PVIterator<isConst>
		PackedVector<T, A>::PVIterator<isConst>::operator++(int)
	{
		PVIterator ret(*this);
		operator++();
		return ret;
	}


	template <typename T, typename A>
	template <bool isConst>
	PackedVector<T, A>::PVIterator<isConst>&
		PackedVector<T, A>::PVIterator<isConst>::operator--()
	{
		do {
			--mIndex;
		}
		while (!mVector->isActive(mIndex) && (mIndex < mVector->mUsedElements));

		return *this;
	}


	template <typename T, typename A>
	template <bool isConst>
	PackedVector<T, A>::PVIterator<isConst>
		PackedVector<T, A>::PVIterator<isConst>::operator--(int)
	{
		PVIterator ret(*this);
		operator--();
		return ret;
	}

}

#endif		// PACKED_VECTOR_HPP
