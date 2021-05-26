#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <memory>
#include <vector>

namespace se::app {

	/**
	 * Class Repository, it provides a single point for storing and accessing
	 * to all the Elements of the given types
	 */
	class Repository
	{
	private:	// Nested types
		struct IRepoTable;
		template <typename KeyType, typename ValueType> struct RepoTable;
		using IRepoTableUPtr = std::unique_ptr<IRepoTable>;

	private:	// Attributes
		/** The number of different RepoTable types */
		static std::size_t sRepoTableTypeCount;

		/** All the RepoTables added to the Repository indexed by their
		 * RepoTable type Id */
		std::vector<IRepoTableUPtr> mRepoTables;

	public:		// Functions
		/** Initializes the Repo so it can hold elements of @tparam ValueType
		 * type indexed by @tparam KeyType
		 * @note it must be called only once before trying to
		 * add, remove or searching elements of the given types */
		template <typename KeyType, typename ValueType>
		void init();

		/** Adds a new element to the Repository
		 *
		 * @param	key the Key with which the new element can be identified in
		 *			the Repository
		 * @param	args the arguments used for creating the element
		 * @return	a pointer to the new element, nullptr if it wasn't
		 *			inserted */
		template <typename KeyType, typename ValueType, typename... Args>
		std::shared_ptr<ValueType> emplace(const KeyType& key, Args&&... args);

		/** Adds the given element to the Repository
		 *
		 * @param	key the Key with which the new element can be identified in
		 *			the Repository
		 * @param	value the element to add to the Repository
		 * @return	a pointer to the new element, nullptr if it wasn't
		 *			inserted */
		template <typename KeyType, typename ValueType>
		std::shared_ptr<ValueType> add(
			const KeyType& key, std::shared_ptr<ValueType> value
		);

		/** Searchs an element with the given key
		 *
		 * @param	key the key used for searching
		 * @return	a pointer to the element found, nullptr if it wasn't
		 *			found */
		template <typename KeyType, typename ValueType>
		std::shared_ptr<ValueType> find(const KeyType& key) const;

		/** Searchs the key of the given Element
		 *
		 * @param	value a pointer to the element to search its key
		 * @param	key where the key will be stored
		 * @return	true if the key was found, false otherwise */
		template <typename KeyType, typename ValueType>
		bool findKey(std::shared_ptr<ValueType> value, KeyType& key) const;

		/** Checks if there is an element with the given key
		 *
		 * @param	key the key used for searching
		 * @return	true if there is a value with the given key, false
		 *			otherwise */
		template <typename KeyType, typename ValueType>
		bool has(const KeyType& key);

		/** Iterates through all the elements of the Repository with the given
		 * @tparam KeyType and @tparam ValueType calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each element */
		template <typename KeyType, typename ValueType, typename F>
		void iterate(F&& callback) const;

		/** Iterates through all the elements of the Repository with the given
		 * @tparam KeyType and @tparam ValueType calling the given
		 * callback function
		 *
		 * @param	callback the function to call for each element */
		template <typename KeyType, typename ValueType, typename F>
		void iterate(F&& callback);

		/** Removes an element with the given key from the Repository
		 *
		 * @param	key the Key with which the element was added to the
		 *			Repository */
		template <typename KeyType, typename ValueType>
		void remove(const KeyType& key);
	private:
		/** @return	the type id of the RepoTable with @tparam KeyType and
		 *			@param ValueType */
		template <typename KeyType, typename ValueType>
		static std::size_t getRepoTableTypeId();

		/** @return	a reference to the RepoTable with the given @tparam KeyType
		 *			and @tparam ValueType */
		template <typename KeyType, typename ValueType>
		RepoTable<KeyType, ValueType>& getRepoTable() const;
	};

}

#include "Repository.hpp"

#endif		// REPOSITORY_H
