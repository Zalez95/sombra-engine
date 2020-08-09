#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

namespace se::utils {

	/** Struct RepoTableTypes, holds metadata about the RepoTableTypes */
	struct Repository::RepoTableTypes
	{
		/** The number of RepoTableTypes */
		static std::size_t sCount;
	};


	/** Struct RepoTableType, it's used for calculating the RepoTable Id
	 * automatically of the table indexed by @tparam KeyType that holds
	 * @tparam ValueType */
	template <typename KeyType, typename ValueType>
	struct Repository::RepoTableType
	{
		/** @return	the RepoTable id of @tparam KeyType and @tparam ValueType */
		static std::size_t getId()
		{
			static std::size_t sRepoTableId = RepoTableTypes::sCount++;
			return sRepoTableId;
		};
	};


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
		auto table = std::make_unique<RepoTable<KeyType, ValueType>>();
		mRepoTables.emplace(RepoTableType<KeyType, ValueType>::getId(), std::move(table));
	}


	template <typename KeyType, typename ValueType>
	std::shared_ptr<ValueType> Repository::add(const KeyType& key, std::unique_ptr<ValueType> value)
	{
		auto [it, inserted] = getRepoTable<KeyType, ValueType>().data.emplace(key, std::move(value));
		if (inserted) {
			return it->second;
		}

		return nullptr;
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

// Private functions
	template <typename KeyType, typename ValueType>
	Repository::RepoTable<KeyType, ValueType>& Repository::getRepoTable()
	{
		return *dynamic_cast<RepoTable<KeyType, ValueType>*>(
			mRepoTables[RepoTableType<KeyType, ValueType>::getId()].get()
		);
	}

}

#endif		// REPOSITORY_HPP
