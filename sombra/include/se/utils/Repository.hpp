#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

#include <unordered_map>

namespace se::utils {

	/**
	 * Class IRepoTable, it's the Interface that every RepoTable must implement
	 */
	struct Repository::IRepoTable
	{
		virtual ~IRepoTable() = default;
	};


	/**
	 * Class RepoTable, it's the IRepoTable that holds the relationship between
	 * the @tparam KeyType keys and the values @tparam ValueType stored
	 */
	template <typename KeyType, typename ValueType>
	struct Repository::RepoTable : public IRepoTable
	{
		/** All the data stored in the RepoTable */
		std::unordered_map<KeyType, std::shared_ptr<ValueType>> data;
	};


	template <typename KeyType, typename ValueType>
	void Repository::init()
	{
		std::size_t id = getRepoTableTypeId<KeyType, ValueType>();
		while (id >= mRepoTables.size()) {
			mRepoTables.emplace_back(nullptr);
		}

		mRepoTables[id] = std::make_unique<RepoTable<KeyType, ValueType>>();
	}


	template <typename KeyType, typename ValueType>
	std::shared_ptr<ValueType> Repository::add(const KeyType& key, std::shared_ptr<ValueType> value)
	{
		auto [it, inserted] = getRepoTable<KeyType, ValueType>().data.emplace(key, std::move(value));
		if (inserted) {
			return it->second;
		}

		return nullptr;
	}


	template <typename KeyType, typename ValueType>
	std::shared_ptr<ValueType> Repository::find(const KeyType& key)
	{
		auto& table = getRepoTable<KeyType, ValueType>();
		auto it = table.data.find(key);
		if (it != table.data.end()) {
			return it->second;
		}

		return nullptr;
	}


	template <typename KeyType, typename ValueType>
	bool Repository::has(const KeyType& key)
	{
		auto& table = getRepoTable<KeyType, ValueType>();
		auto it = table.data.find(key);
		if (it != table.data.end()) {
			return true;
		}

		return false;
	}


	template <typename KeyType, typename ValueType, typename F>
	void Repository::iterate(F&& callback)
	{
		auto& table = getRepoTable<KeyType, ValueType>();
		for (auto& [key, value] : table.data) {
			callback(key, value);
		}
	}


	template <typename KeyType, typename ValueType>
	void Repository::remove(const KeyType& key)
	{
		auto& table = getRepoTable<KeyType, ValueType>();
		auto it = table.data.find(key);
		if (it != table.data.end()) {
			table.data.erase(it);
		}
	}

// Private functions
	template <typename KeyType, typename ValueType>
	std::size_t Repository::getRepoTableTypeId()
	{
		static std::size_t sRepoTableId = sRepoTableTypeCount++;
		return sRepoTableId;
	}


	template <typename KeyType, typename ValueType>
	Repository::RepoTable<KeyType, ValueType>& Repository::getRepoTable()
	{
		return *static_cast<RepoTable<KeyType, ValueType>*>(
			mRepoTables[getRepoTableTypeId<KeyType, ValueType>()].get()
		);
	}

}

#endif		// REPOSITORY_HPP
