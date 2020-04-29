#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

namespace se::utils {

	template <typename KeyType, typename ValueType>
	std::shared_ptr<ValueType> Repository<KeyType, ValueType>::add(const KeyType& key, std::unique_ptr<ValueType> value)
	{
		auto [it, inserted] = mData.emplace(key, std::move(value));
		if (inserted) {
			return it->second;
		}

		return nullptr;
	}


	template <typename KeyType, typename ValueType>
	void Repository<KeyType, ValueType>::remove(const KeyType& key)
	{
		auto it = mData.find(key);
		if (it != mData.end()) {
			mData.erase(it);
		}
	}


	template <typename KeyType, typename ValueType>
	std::shared_ptr<ValueType> Repository<KeyType, ValueType>::find(const KeyType& key)
	{
		auto it = mData.find(key);
		if (it != mData.end()) {
			return it->second;
		}

		return nullptr;
	}

}

#endif		// REPOSITORY_HPP
