#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

#include <mutex>
#include "../utils/MathUtils.h"

namespace se::app {

	/**
	 * Class Resource, it holds a resource of type @tparam T and its metadata
	 */
	template <typename T>
	struct Repository::Resource
	{
		/** A pointer to the Resource */
		std::shared_ptr<T> resource;

		/** The name of the Resource */
		std::string name;

		/** The index of the linked Scene file where the Resource is stored,
		 * If it's negative then the Resource is located in the same
		 * Scene file where the MetaResource is located */
		int linkedFile = -1;

		/** The path where the resource is located. If there are multiple
		 * paths they will be separated by pipes (|), if it isn't located in any
		 * file it will be empty. */
		std::string path;

		/** The number of Users of the current Resource. The last bit is used
		 * as a "Fake" user, for preventing the Resource from being Removed
		 * even if has no real users. */
		std::size_t userCount = 0;
	};


	/**
	 * Class IRepoTable, it's the Interface that every RepoTable must
	 * implement
	 */
	struct Repository::IRepoTable
	{
		virtual ~IRepoTable() = default;
	};


	/**
	 * Class RepoTable, it's the IRepoTable that holds all the values of
	 * type @tparam T
	 */
	template <typename T>
	struct Repository::RepoTable : public Repository::IRepoTable
	{
		/** All the data stored in the RepoTable */
		utils::PackedVector<Resource<T>> data;

		/** The function used for clonying a Resource */
		CloneCallback<T> cloneCallback;

		/** The mutex used for protecting the Repository */
		mutable std::recursive_mutex mutex;
	};


	template <typename T>
	std::size_t Repository::ResourceRef<T>::HashFunc::operator()(const ResourceRef& ref) const
	{
		std::size_t ret = 0;
		utils::hash_combine(ret, ref.mParent);
		utils::hash_combine(ret, ref.mIndex);
		return ret;
	}


	template <typename T>
	Repository::ResourceRef<T>::ResourceRef(Repository* parent, std::size_t index) :
		mParent(parent), mIndex(index)
	{
		if (mParent) {
			mParent->addUser<T>(mIndex);
		}
	}


	template <typename T>
	Repository::ResourceRef<T>::ResourceRef(const ResourceRef& other) :
		mParent(other.mParent), mIndex(other.mIndex)
	{
		if (mParent) {
			mParent->addUser<T>(mIndex);
		}
	}


	template <typename T>
	Repository::ResourceRef<T>::ResourceRef(ResourceRef&& other) :
		mParent(std::move(other.mParent)), mIndex(std::move(other.mIndex))
	{
		other.mParent = nullptr;
	}


	template <typename T>
	Repository::ResourceRef<T>::~ResourceRef()
	{
		if (mParent) {
			mParent->removeUser<T>(mIndex);
		}
	}


	template <typename T>
	Repository::ResourceRef<T>& Repository::ResourceRef<T>::operator=(const ResourceRef& other)
	{
		if (mParent) {
			mParent->removeUser<T>(mIndex);
		}

		mParent = other.mParent;
		mIndex = other.mIndex;

		if (mParent) {
			mParent->addUser<T>(mIndex);
		}

		return *this;
	}


	template <typename T>
	Repository::ResourceRef<T>& Repository::ResourceRef<T>::operator=(ResourceRef&& other)
	{
		if (mParent) {
			mParent->removeUser<T>(mIndex);
		}

		mParent = std::move(other.mParent);
		mIndex = std::move(other.mIndex);
		other.mParent = nullptr;
		return *this;
	}


	template <typename T>
	void Repository::init(const CloneCallback<T>& cloneCB)
	{
		std::size_t id = getRepoTableTypeId<T>();
		while (id >= mRepoTables.size()) {
			mRepoTables.emplace_back(nullptr);
		}

		auto table = std::make_unique<RepoTable<T>>();
		table->cloneCallback = cloneCB;
		mRepoTables[id] = std::move(table);
	}


	template <typename T>
	void Repository::clear()
	{
		getRepoTable<T>().data.clear();
	}


	template <typename T, typename... Args>
	Repository::ResourceRef<T> Repository::emplace(Args&&... args)
	{
		auto it = getRepoTable<T>().data.emplace();
		it->resource = std::make_shared<T>(std::forward<Args>(args)...);

		return ResourceRef<T>(this, it.getIndex());
	}


	template <typename T>
	Repository::ResourceRef<T> Repository::insert(const std::shared_ptr<T>& value, const char* name)
	{
		auto it = getRepoTable<T>().data.emplace();
		it->resource = value;
		it->name = name;

		return ResourceRef<T>(this, it.getIndex());
	}


	template <typename T>
	Repository::ResourceRef<T> Repository::clone(const ResourceRef<T>& resource)
	{
		ResourceRef<T> ret;

		const auto& table = getRepoTable<T>();
		if (table.cloneCallback) {
			auto value = table.cloneCallback(*resource);
			if (value) {
				ret = insert<T>(std::move(value));
				ret.setName( resource.getName() );
				ret.setPath( resource.getPath() );
				if (resource.isLinked()) {
					ret.setLinkedFile( resource.getLinkedFile() );
				}
			}
		}

		return ret;
	}


	template <typename T, typename F>
	Repository::ResourceRef<T> Repository::find(F&& compare) const
	{
		const auto& table = getRepoTable<T>();

		std::scoped_lock lock(table.mutex);
		for (auto it = table.data.begin(); it != table.data.end(); ++it) {
			ResourceRef<T> ref(const_cast<Repository*>(this), it.getIndex());
			if (compare(ref)) {
				return ref;
			}
		}

		return ResourceRef<T>();
	}


	template <typename T>
	Repository::ResourceRef<T> Repository::findByName(const char* name) const
	{
		return find<T>([&](const ResourceRef<T>& ref) {
			return ref.getName() == name;
		});
	}


	template <typename T>
	Repository::ResourceRef<T> Repository::findResource(const T* resource) const
	{
		return find<T>([&](const ResourceRef<T>& ref) {
			return ref.get().get() == resource;
		});
	}


	template <typename T, typename F>
	void Repository::iterate(F&& callback) const
	{
		const auto& table = getRepoTable<T>();

		std::scoped_lock lock(table.mutex);
		for (auto it = table.data.begin(); it != table.data.end(); ++it) {
			callback(ResourceRef<T>(const_cast<Repository*>(this), it.getIndex()));
		}
	}

// Private functions
	template <typename T>
	std::shared_ptr<T> Repository::get(std::size_t index) const
	{
		auto& table = getRepoTable<T>();
		std::scoped_lock lock(table.mutex);
		return table.data[index].resource;
	}


	template <typename T>
	std::string Repository::getName(std::size_t index) const
	{
		auto& table = getRepoTable<T>();
		std::scoped_lock lock(table.mutex);
		return table.data[index].name;
	}


	template <typename T>
	void Repository::setName(std::size_t index, const std::string& name)
	{
		auto& table = getRepoTable<T>();
		std::scoped_lock lock(table.mutex);
		table.data[index].name = name;
	}


	template <typename T>
	int Repository::getLinkedFile(std::size_t index) const
	{
		auto& table = getRepoTable<T>();
		std::scoped_lock lock(table.mutex);
		return table.data[index].linkedFile;
	}


	template <typename T>
	void Repository::setLinkedFile(std::size_t index, int linkedFile)
	{
		auto& table = getRepoTable<T>();
		std::scoped_lock lock(table.mutex);
		table.data[index].linkedFile = linkedFile;
	}


	template <typename T>
	std::string Repository::getPath(std::size_t index) const
	{
		auto& table = getRepoTable<T>();
		std::scoped_lock lock(table.mutex);
		return table.data[index].path;
	}


	template <typename T>
	void Repository::setPath(std::size_t index, const std::string& path)
	{
		auto& table = getRepoTable<T>();
		std::scoped_lock lock(table.mutex);
		table.data[index].path = path;
	}


	template <typename T>
	std::size_t Repository::getUserCount(std::size_t index)
	{
		auto& table = getRepoTable<T>();
		std::scoped_lock lock(table.mutex);
		return table.data[index].userCount;
	}


	template <typename T>
	void Repository::addUser(std::size_t index)
	{
		auto& table = getRepoTable<T>();
		std::scoped_lock lock(table.mutex);
		table.data[index].userCount++;
	}


	template <typename T>
	bool Repository::hasFakeUser(std::size_t index)
	{
		auto& table = getRepoTable<T>();
		std::scoped_lock lock(table.mutex);
		return static_cast<bool>(table.data[index].userCount >> (8 * sizeof(size_t) - 1));
	}


	template <typename T>
	void Repository::setFakeUser(std::size_t index, bool fakeUser)
	{
		auto& table = getRepoTable<T>();
		std::scoped_lock lock(table.mutex);
		if (fakeUser) {
			std::size_t fakeBit = 1;
			fakeBit <<= 8 * sizeof(size_t) - 1;
			table.data[index].userCount |= fakeBit;
		}
		else {
			table.data[index].userCount = (table.data[index].userCount << 1) >> 1;
			if (table.data[index].userCount == 0) {
				table.data.erase( table.data.begin().setIndex(index) );
			}
		}
	}


	template <typename T>
	void Repository::removeUser(std::size_t index)
	{
		auto& table = getRepoTable<T>();
		std::scoped_lock lock(table.mutex);
		if (--table.data[index].userCount == 0) {
			table.data.erase( table.data.begin().setIndex(index) );
		}
	}


	template <typename T>
	std::size_t Repository::getRepoTableTypeId()
	{
		static std::size_t sRepoTableId = sRepoTableTypeCount++;
		return sRepoTableId;
	}


	template <typename T>
	Repository::RepoTable<T>& Repository::getRepoTable() const
	{
		return *static_cast<RepoTable<T>*>(
			mRepoTables[getRepoTableTypeId<T>()].get()
		);
	}

}

#endif		// REPOSITORY_HPP
