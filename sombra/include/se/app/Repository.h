#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <memory>
#include <vector>
#include <string>
#include "../utils/PackedVector.h"

namespace se::app {

	/**
	 * Class Resource, it holds a resource of type @tparam T and
	 * its metadata
	 */
	template <typename T>
	class Resource
	{
	private:	// Attribute
		friend class Repository;

		/** A pointer to the Resource */
		std::shared_ptr<T> mResource;

		/** The name of the Resource */
		std::string mName;

		/** The index of the linked Scene file where the Resource is stored,
		 * If it's negative then the Resource is located in the same
		 * Scene file where the MetaResource is located */
		int mLinkedFile = -1;

		/** The path where the resource is located. If there are multiple
		 * paths they will be separated by pipes (|), if it isn't located in any
		 * file it will be empty. */
		std::string mPath;

		/** The number of Users of the current Resource. The last bit is used
		 * as a "Fake" user, for preventing the Resource from being Removed
		 * even if has no real users. */
		std::size_t mUserCount = 0;

	public:		// Functions
		/** @return	a pointer to the Resource */
		const std::shared_ptr<T>& getResource() const { return mResource; };

		/** @return	the name of the Resource */
		const char* getName() const { return mName.c_str(); };

		/** Sets the name of the Resource
		 *
		 * @param	name the new Name of the Resource */
		void setName(const char* name) { mName = name; };

		/** @return	true if the Resource is stored in a linked Scene file,
		 *			false otherwise */
		bool isLinked() const { return mLinkedFile >= 0; };

		/** @return	the index of the linked Scene file of the Resource */
		std::size_t getLinkedFile() const { return mLinkedFile; };

		/** Sets the index of the linked Scene file of the Resource
		 *
		 * @param	linkedFile the new index */
		void setLinkedFile(std::size_t linkedFile)
		{ mLinkedFile = static_cast<int>(linkedFile); };

		/** Removes the linked file from the Resource */
		void unLink() { mLinkedFile = -1; };

		/** @return	the path where the Resource is located */
		const char* getPath() const { return mPath.c_str(); };

		/** Sets the path where the Resource is located
		 *
		 * @param	path the new path */
		void setPath(const char* path) { mPath = path; };
	};


	/**
	 * Class Repository, it provides a single point for storing and accessing
	 * to all the Elements of the given types. Resources are automatically
	 * removed from the Repository if there are no references pointing to them.
	 */
	class Repository
	{
	public:		// Nested types
		/** Class ResourceRef, it's a reference to a Resource of the
		 * Repository */
		template <typename T>
		class ResourceRef
		{
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
			 * @return	true if both ResourceRefs are different, false
			 *			otherwise */
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
			{ return getResource().getResource(); };

			/** @return	the Repository where the Resource is located */
			Repository* getParent() const
			{ return mParent; };

			/** @return	a reference to the Resource structure stored */
			Resource<T>& getResource()
			{ return mParent->at<T>(mIndex); };
			const Resource<T>& getResource() const
			{ return mParent->at<T>(mIndex); };

			/** @return	the number of users of the Resource */
			std::size_t getUserCount() const;

			/** @return	true if the Resource is preserved even if it has no
			 *			users, false otherwise */
			bool hasFakeUser() const;

			/** Adds a Fake user to the Resource so it will be preserved even if
			 * it has no real users
			 *
			 *@param	fakeUser true if we want to add a fake user, false if we
			*			want to remove it */
			void setFakeUser(bool fakeUser = true);
		};

	private:
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
		 * @note	it must be called only once before trying to add, remove or
		 *			search elements of the given types. On destroy, the objects
		 *			will be cleared in reverse order than they were
		 *			initialized */
		template <typename T>
		void init();

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
		/** Returns the element with @tparam T type located at the given index
		 *
		 * @param	index the index of the element to retrieve
		 * @return	the Resource element */
		template <typename T>
		Resource<T>& at(std::size_t index);

		/** Returns the element with @tparam T type located at the given index
		 *
		 * @param	index the index of the element to retrieve
		 * @return	the Resource element */
		template <typename T>
		const Resource<T>& at(std::size_t index) const;

		/** Adds a user to the element with @tparam T type located at the given
		 * index
		 *
		 * @param	index the index of the element to add a user */
		template <typename T>
		void addUser(std::size_t index);

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

}

#include "Repository.hpp"

#endif		// REPOSITORY_H
