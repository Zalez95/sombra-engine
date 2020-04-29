#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <memory>
#include <unordered_map>

namespace se::utils {

	/**
	 * Class Repository, it holds all the elements of the given type loaded by
	 * the Application and provides a single point for accessing to them
	 */
	template <typename KeyType, typename ValueType>
	class Repository
	{
	private:	// Attributes
		/** All the data stored in the Repository */
		std::unordered_map<KeyType, std::shared_ptr<ValueType>> mData;

	public:		// Functions
		/** Adds the given element to the Repository
		 *
		 * @param	key the Key with which the new element can be identified in
		 *			the Repository
		 * @param	value the element to add to the Repository
		 * @return	a pointer to the new element, nullptr if it wasn't
		 *			inserted */
		std::shared_ptr<ValueType> add(
			const KeyType& key, std::unique_ptr<ValueType> value
		);

		/** Removes an element with the given key from the Repository
		 *
		 * @param	key the Key with which the element was added to the
		 *			Repository */
		void remove(const KeyType& key);

		/** Searchs an element with the given key value
		 *
		 * @param	key the key value used for searching
		 * @return	a pointer to the element found, nullptr if it wasn't
		 *			found */
		std::shared_ptr<ValueType> find(const KeyType& key);
	};

}

#include "Repository.hpp"

#endif		// REPOSITORY_H
