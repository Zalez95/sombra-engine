#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "../utils/PackedVector.h"

namespace se::app {

	/**
	 * Class Repository, it provides a single point for storing and accessing
	 * to all the Elements of the given types. Resources are automatically
	 * removed from the Repository if there are no references pointing to them.
	 */
	class Repository
	{
	public:		// Nested types
		template <typename T> class ResourceRef;
		template <typename T> using CloneCallback =
			std::function<std::unique_ptr<T>(const T&)>;
	private:
		template <typename T> struct Resource;
		struct IRepoTable;
		template <typename T> struct RepoTable;
		using IRepoTableUPtr = std::unique_ptr<IRepoTable>;

	private:	// Attributes
		/** The number of different RepoTable types */
		static std::size_t sRepoTableTypeCount;

		/** All the RepoTables added to the Repository indexed by their
		 * RepoTable type Id */
		std::vector<IRepoTableUPtr> mRepoTables;

	public:		// Functions
		/** Creates a new Repository */
		Repository() = default;
		Repository(const Repository& other) = default;
		Repository(Repository&& other) = default;

		/** Class destructor */
		~Repository();

		/** Assignment operator */
		Repository& operator=(const Repository& other) = default;
		Repository& operator=(Repository&& other) = default;

		/** Initializes the Repository so it can hold elements of @tparam T type
		 *
		 * @param	cloneCB the function used for clonying the elements
		 * @note	it must be called only once before trying to add, remove or
		 *			search elements of the given types. On destroy, the objects
		 *			will be cleared in reverse order than they were
		 *			initialized */
		template <typename T>
		void init(const CloneCallback<T>& cloneCB = {});

		/** Removes all the elements with @tparam T type from the Repository
		 * @note	all the references to those objects will be invalidated */
		template <typename T>
		void clear();

		/** Adds a new element to the Repository
		 *
		 * @param	args the arguments used for creating the element
		 * @return	a ResourceRef to the new element */
		template <typename T, typename... Args>
		ResourceRef<T> emplace(Args&&... args);

		/** Adds the given element to the Repository
		 *
		 * @param	value a pointer to the element to add to the Repository
		 * @param	name the name of the new element
		 * @return	a ResourceRef to the new element */
		template <typename T>
		ResourceRef<T> insert(
			const std::shared_ptr<T>& value, const char* name = ""
		);

		/** Creates a copy of the given Resource
		 *
		 * @param	resource a ResourceRef to the Resource to create a copy of
		 * @return	a ResourceRef to the new copy, it will be invalid if the
		 *			element couldn't be cloned
		 * @see		setCloneCallback */
		template <typename T>
		ResourceRef<T> clone(const ResourceRef<T>& resource);

		/** Searchs an element in the Repository with the given @tparam T type
		 * using the given comparison function
		 *
		 * @param	compare the function to use for comparison
		 * @return	a ResourceRef to the element found, it will be invalid if
		 *			it doesn't find any element */
		template <typename T, typename F>
		ResourceRef<T> find(F&& compare) const;

		/** Searchs an element in the Repository with the given @tparam T type
		 * by its Resource name
		 *
		 * @param	name the name of the Resource to search
		 * @return	a ResourceRef to the element found, it will be invalid if
		 *			it doesn't find any element */
		template <typename T>
		ResourceRef<T> findByName(const char* name) const;

		/** Searchs an element in the Repository with the given @tparam T type
		 * by its pointer
		 *
		 * @param	resource a pointer to the Resource to search
		 * @return	a ResourceRef to the element found, it will be invalid if
		 *			it doesn't find any element */
		template <typename T>
		ResourceRef<T> findResource(const T* resource) const;

		/** Iterates through all the elements of the Repository with the given
		 * @tparam T type calling the given callback function
		 *
		 * @param	callback the function to call for each element */
		template <typename T, typename F>
		void iterate(F&& callback) const;
	private:
		/** @return	a shared_ptr to the Resource content */
		template <typename T>
		std::shared_ptr<T> get(std::size_t index) const;

		/** Returns the name of the Resource
		 *
		 * @param	index the index of the element to get its name
		 * @return	the name of the Resource */
		template <typename T>
		std::string getName(std::size_t index) const;

		/** Sets the name of the Resource
		 *
		 * @param	index the index of the element to set its name
		 * @param	name the new name of the Resource */
		template <typename T>
		void setName(std::size_t index, const std::string& name);

		/** Returns the linked file of the Resource
		 *
		 * @param	index the index of the element to get its linked file
		 * @return	the index of the linked Scene file of the Resource */
		template <typename T>
		int getLinkedFile(std::size_t index) const;

		/** Sets the index of the linked Scene file of the Resource
		 *
		 * @param	index the index of the element to set its linked file
		 * @param	linkedFile the new index */
		template <typename T>
		void setLinkedFile(std::size_t index, int linkedFile);

		/** Returns the path of the Resource
		 *
		 * @param	index the index of the element to get its path
		 * @return	the path of the Resource */
		template <typename T>
		std::string getPath(std::size_t index) const;

		/** Sets the path of the Resource
		 *
		 * @param	index the index of the element to set its path
		 * @param	path the new path of the Resource */
		template <typename T>
		void setPath(std::size_t index, const std::string& path);

		/** Returns the number of users of the Resource with @tparam T located
		 * at the given index
		 *
		 * @param	index the index of the element to get its user count
		 * @return	the number of users */
		template <typename T>
		std::size_t getUserCount(std::size_t index);

		/** Adds a user to the element with @tparam T type located at the given
		 * index
		 *
		 * @param	index the index of the element to add a user */
		template <typename T>
		void addUser(std::size_t index);

		/** Returns the Fake user of the Resource with @tparam T located
		 * at the given index
		 *
		 * @param	index the index of the element to get its fake user
		 * @return	true if the Resource has a fake user, false otherwise */
		template <typename T>
		bool hasFakeUser(std::size_t index);

		/** Sets the Fake user to the Resource with @tparam T located
		 * at the given index
		 *
		 * @param	index the index of the element to set its fake user
		 * @param	fakeUser true if we want to add a fake user, false if we
		 *			want to remove it */
		template <typename T>
		void setFakeUser(std::size_t index, bool fakeUser);

		/** Removes a user from the element with @tparam T type located at the
		 * given index
		 *
		 * @param	index the index of the element to remove a user
		 * @note	if an element has zero elements it will be removed from the
		 *			Repository */
		template <typename T>
		void removeUser(std::size_t index);

		/** @return	the type id of the RepoTable with @tparam T type */
		template <typename T>
		static std::size_t getRepoTableTypeId();

		/** @return	a reference to the RepoTable with the given
		 *			@tparam T type */
		template <typename T>
		RepoTable<T>& getRepoTable() const;
	};


	/** Class ResourceRef, it's a reference to a Resource of the Repository */
	template <typename T>
	class Repository::ResourceRef
	{
	public:		// Nested types
		/** Struct HashFunc, used for calculating the hash value of
		 * ResourceRefs */
		struct HashFunc
		{
			/** Calculates the hash value of the given ResourceRef
			 *
			 * @param	ref the ResourceRef to calculate its hash value
			 * @return	the hash value */
			std::size_t operator()(const ResourceRef& ref) const;
		};

	private:	// Attributes
		/** The Repository that holds the resource referenced */
		Repository* mParent;

		/** The index to the resource referenced in the Repository */
		std::size_t mIndex;

	public:
		/** Creates a new ResourceRef
		 *
		 * @param	parent the Repository that holds the resource
		 *			referenced
		 * @param	index the index to the resource referenced in the
		 *			Repository */
		ResourceRef(Repository* parent = nullptr, std::size_t index = 0);
		ResourceRef(const ResourceRef& other);
		ResourceRef(ResourceRef&& other);

		/** Class destructor */
		~ResourceRef();

		/** Assignment operator */
		ResourceRef& operator=(const ResourceRef& other);
		ResourceRef& operator=(ResourceRef&& other);

		/** @return	true if the ResourceRef reference to any element,
		 *			false otherwise */
		operator bool() const { return mParent; };

		/** Compares the given ResourceRefs
		 *
		 * @param	ref1 the first ResourceRef to compare
		 * @param	ref2 the second ResourceRef to compare
		 * @return	true if both ResourceRefs are equal, false otherwise */
		friend bool operator==(
			const ResourceRef& ref1, const ResourceRef& ref2
		) { return (ref1.mParent == ref2.mParent)
				&& (ref1.mIndex == ref2.mIndex); };

		/** Compares the given ResourceRefs
		 *
		 * @param	ref1 the first ResourceRef to compare
		 * @param	ref2 the second ResourceRef to compare
		 * @return	true if both ResourceRefs are different, false otherwise */
		friend bool operator!=(
			const ResourceRef& ref1, const ResourceRef& ref2
		) { return !(ref1 == ref2); };

		/** @return	a reference to the Resource content */
		const T& operator*() const { return *get(); };
		T& operator*() { return *get(); };

		/** @return	a raw pointer to the Resource content */
		const T* operator->() const { return get().get(); };
		T* operator->() { return get().get(); };

		/** @return	a shared_ptr to the Resource content */
		std::shared_ptr<T> get() const
		{ return mParent? mParent->get<T>(mIndex) : nullptr; };

		/** @return	the Repository that holds the resource referenced */
		Repository* getParent() const { return mParent; };

		/** @return	the name of the Resource */
		std::string getName() const
		{ return mParent? mParent->getName<T>(mIndex) : ""; };

		/** Sets the name of the Resource
		 *
		 * @param	name the new Name of the Resource
		 * @return	a reference to the current ResourceRef */
		ResourceRef& setName(const std::string& name)
		{
			if (mParent) {
				mParent->setName<T>(mIndex, name);
			}
			return *this;
		};

		/** @return	true if the Resource is stored in a linked Scene file,
		 *			false otherwise */
		bool isLinked() const
		{ return mParent? mParent->getLinkedFile<T>(mIndex) >= 0 : false; };

		/** @return	the index of the linked Scene file of the Resource */
		int getLinkedFile() const
		{ return mParent? mParent->getLinkedFile<T>(mIndex) : -1; };

		/** Sets the index of the linked Scene file of the Resource
		 *
		 * @param	linkedFile the new index
		 * @return	a reference to the current ResourceRef */
		ResourceRef& setLinkedFile(int linkedFile)
		{
			if (mParent) {
				mParent->setLinkedFile<T>(mIndex, linkedFile);
			}
			return *this;
		};

		/** Removes the linked file from the Resource
		 *
		 * @return	a reference to the current ResourceRef */
		ResourceRef& unLink()
		{
			if (mParent) {
				mParent->setLinkedFile<T>(mIndex, -1);
			}
			return *this;
		};

		/** @return	the path where the Resource is located */
		std::string getPath() const
		{ return mParent? mParent->getPath<T>(mIndex) : ""; };

		/** Sets the path where the Resource is located
		 *
		 * @param	path the new path
		 * @return	a reference to the current ResourceRef */
		ResourceRef& setPath(const std::string& path)
		{
			if (mParent) {
				mParent->setPath<T>(mIndex, path);
			}
			return *this;
		};

		/** @return	the number of users of the Resource */
		std::size_t getUserCount() const
		{ return mParent? mParent->getUserCount<T>(mIndex) : 0; };

		/** @return	true if the Resource is preserved even if it has no users,
		 *			false otherwise */
		bool hasFakeUser() const
		{ return mParent? mParent->hasFakeUser<T>(mIndex) : false; };

		/** Adds a Fake user to the Resource so it will be preserved even if
		 * it has no real users
		 *
		 *@param	fakeUser true if we want to add a fake user, false if we
		 *			want to remove it
		 * @return	a reference to the current ResourceRef */
		ResourceRef& setFakeUser(bool fakeUser = true)
		{
			if (mParent) {
				mParent->setFakeUser<T>(mIndex, fakeUser);
			}
			return *this;
		};
	};

}

#include "Repository.hpp"

#endif		// REPOSITORY_H
