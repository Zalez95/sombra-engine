#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

#include <algorithm>

namespace se::utils {

	template <typename T>
	Repository<T>& Repository<T>::getInstance()
	{
		static Repository<T> instance;
		return instance;
	}


	template <typename T>
	void Repository<T>::add(T element)
	{
		if (element) {
			mData.push_back(element);
		}
	}


	template <typename T>
	void Repository<T>::remove(T element)
	{
		mData.erase(std::remove(mData.begin(), mData.end(), element), mData.end());
	}


	template <typename T>
	void Repository<T>::clear()
	{
		mData.clear();
	}


	template <typename T>
	template <class F>
	bool Repository<T>::has(const F& f) const
	{
		return (std::find_if(mData.begin(), mData.end(), f) != mData.end());
	}


	template <typename T>
	template <class F>
	const T& Repository<T>::get(const F& f) const
	{
		return *std::find_if(mData.begin(), mData.end(), f);
	}

}

#endif		// REPOSITORY_HPP
