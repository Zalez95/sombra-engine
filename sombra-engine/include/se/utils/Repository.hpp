#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

namespace se::utils {

	template <typename T, typename SizeType>
	Repository<T, SizeType>::Repository(SizeType maxElements) : mMaxElements(maxElements)
	{
		mData.reserve(mMaxElements);
		mRefCounts.reserve(mMaxElements);
	}


	template <typename T, typename SizeType>
	template <typename... Args>
	typename Repository<T, SizeType>::Reference Repository<T, SizeType>::add(Args&&... args)
	{
		Reference ret(this);

		for (std::size_t i = 0; i < mData.size(); ++i) {
			if (mRefCounts[i] == 0) {
				mData[i] = T(std::forward<Args>(args)...);
				ret = Reference(this, static_cast<SizeType>(i));
			}
		}

		if (!ret) {
			SizeType currentSize = static_cast<SizeType>(mData.size());
			if (currentSize < mMaxElements) {
				mData.emplace_back(std::forward<Args>(args)...);
				mRefCounts.emplace_back(0);
				ret = Reference(this, currentSize);
			}
		}

		return ret;
	}


	template <typename T, typename SizeType>
	typename Repository<T, SizeType>::Reference Repository<T, SizeType>::find(const SearchCallback& callback)
	{
		Reference ret(this);

		for (std::size_t i = 0; i < mData.size(); ++i) {
			if ((mRefCounts[i] > 0) && callback(mData[i])) {
				ret = Reference(this, static_cast<SizeType>(i));
			}
		}

		return ret;
	}


	template <typename T, typename SizeType>
	void Repository<T, SizeType>::incrementRefCount(SizeType index)
	{
		if (index < mMaxElements) {
			mRefCounts[index]++;
		}
	}


	template <typename T, typename SizeType>
	void Repository<T, SizeType>::decrementRefCount(SizeType index)
	{
		if (index < mMaxElements) {
			mRefCounts[index]--;
			if (mRefCounts[index] <= 0) {
				mData[index] = T();
			}
		}
	}


	template <typename T, typename SizeType>
	Repository<T, SizeType>::Reference::Reference(Repository* repository) :
		mRepository(repository), mIndex(0)
	{
		if (mRepository) {
			mIndex = mRepository->mMaxElements;
		}
	}


	template <typename T, typename SizeType>
	Repository<T, SizeType>::Reference::Reference(Repository* repository, SizeType index) :
		mRepository(repository), mIndex(index)
	{
		if (mRepository) {
			mRepository->incrementRefCount(mIndex);
		}
	}


	template <typename T, typename SizeType>
	Repository<T, SizeType>::Reference::Reference(const Reference& other) :
		mRepository(other.mRepository), mIndex(other.mIndex)
	{
		if (mRepository) {
			mRepository->incrementRefCount(mIndex);
		}
	}


	template <typename T, typename SizeType>
	Repository<T, SizeType>::Reference::Reference(Reference&& other) :
		mRepository(other.mRepository), mIndex(other.mIndex)
	{
		if (mRepository) {
			other.mIndex = mRepository->mMaxElements;
		}
	}


	template <typename T, typename SizeType>
	Repository<T, SizeType>::Reference::~Reference()
	{
		if (mRepository) {
			mRepository->decrementRefCount(mIndex);
		}
	}


	template <typename T, typename SizeType>
	typename Repository<T, SizeType>::Reference& Repository<T, SizeType>::Reference::operator=(const Reference& other)
	{
		if (mRepository) {
			mRepository->decrementRefCount(mIndex);
		}

		mRepository = other.mRepository;
		mIndex = other.mIndex;

		if (mRepository) {
			mRepository->incrementRefCount(mIndex);
		}

		return *this;
	}


	template <typename T, typename SizeType>
	typename Repository<T, SizeType>::Reference& Repository<T, SizeType>::Reference::operator=(Reference&& other)
	{
		if (mRepository) {
			mRepository->decrementRefCount(mIndex);
		}

		mRepository = other.mRepository;
		mIndex = other.mIndex;

		if (mRepository) {
			other.mIndex = mRepository->mMaxElements;
		}

		return *this;
	}

}

#endif		// REPOSITORY_HPP
