#ifndef REPOSITORY_HPP
#define REPOSITORY_HPP

namespace se::app {

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
	};


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
	std::size_t Repository::ResourceRef<T>::getUserCount() const
	{
		if (mParent) {
			return (mParent->at<T>(mIndex).mUserCount << 1) >> 1;
		}
		else {
			return 0;
		}
	}


	template <typename T>
	bool Repository::ResourceRef<T>::hasFakeUser() const
	{
		if (mParent) {
			std::size_t fakeBit = 1;
			fakeBit <<= 8 * sizeof(size_t) - 1;
			return (mParent->at<T>(mIndex).mUserCount & fakeBit) > 0;
		}
		else {
			return false;
		}
	}


	template <typename T>
	void Repository::ResourceRef<T>::setFakeUser(bool fakeUser)
	{
		if (mParent) {
			mParent->setFakeUser<T>(mIndex, fakeUser);
		}
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
		it->mResource = std::make_shared<T>(std::forward<Args>(args)...);

		return ResourceRef<T>(this, it.getIndex());
	}


	template <typename T>
	Repository::ResourceRef<T> Repository::insert(const std::shared_ptr<T>& value, const char* name)
	{
		auto it = getRepoTable<T>().data.emplace();
		it->mResource = value;
		it->mName = name;

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
				ret.getResource().setName( resource.getResource().getName() );
				ret.getResource().setPath( resource.getResource().getPath() );
				if (resource.getResource().isLinked()) {
					ret.getResource().setLinkedFile( resource.getResource().getLinkedFile() );
				}
			}
		}

		return ret;
	}


	template <typename T, typename F>
	Repository::ResourceRef<T> Repository::find(F&& compare) const
	{
		const auto& table = getRepoTable<T>();
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
		return find<T>([name](const ResourceRef<T>& ref) {
			return ref.getResource().mName == name;
		});
	}



	template <typename T>
	Repository::ResourceRef<T> Repository::findResource(const T* resource) const
	{
		return find<T>([resource](const ResourceRef<T>& ref) {
			return ref.get().get() == resource;
		});
	}


	template <typename T, typename F>
	void Repository::iterate(F&& callback) const
	{
		const auto& table = getRepoTable<T>();
		for (auto it = table.data.begin(); it != table.data.end(); ++it) {
			callback(ResourceRef<T>(const_cast<Repository*>(this), it.getIndex()));
		}
	}

// Private functions
	template <typename T>
	Resource<T>& Repository::at(std::size_t index)
	{
		return getRepoTable<T>().data[index];
	}


	template <typename T>
	const Resource<T>& Repository::at(std::size_t index) const
	{
		return getRepoTable<T>().data[index];
	}


	template <typename T>
	void Repository::addUser(std::size_t index)
	{
		getRepoTable<T>().data[index].mUserCount++;
	}


	template <typename T>
	void Repository::setFakeUser(std::size_t index, bool fakeUser)
	{
		auto& table = getRepoTable<T>();

		if (fakeUser) {
			std::size_t fakeBit = 1;
			fakeBit <<= 8 * sizeof(size_t) - 1;
			table.data[index].mUserCount |= fakeBit;
		}
		else {
			table.data[index].mUserCount = (table.data[index].mUserCount << 1) >> 1;
			if (table.data[index].mUserCount == 0) {
				table.data.erase( table.data.begin().setIndex(index) );
			}
		}
	}


	template <typename T>
	void Repository::removeUser(std::size_t index)
	{
		auto& table = getRepoTable<T>();
		if (--table.data[index].mUserCount == 0) {
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
