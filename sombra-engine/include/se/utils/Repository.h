#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <vector>

namespace se::utils {

	/**
	 * Class Repository, it holds all the elements of the given type loaded by
	 * the Application and provides a single point for accessing to them
	 */
	template <typename T>
	class Repository
	{
	private:	// Attributes
		/** All the data stored in the Repository */
		std::vector<T> mData;

	public:
		/** Copy constructor */
		Repository(const Repository&) = delete;

		/** Copy assignment operator */
		void operator=(const Repository&) = delete;

		/** @return	the only instance of the Repository */
		static Repository& getInstance();

		/** Adds the given element to the Repository
		 *
		 * @param	element a pointer to the element to add */
		void add(T element);

		/** Removes the given element from the Repository
		 *
		 * @param	element a pointer to the element to add */
		void remove(T element);

		/** Removes all the inserted elements in the Repository */
		void clear();

		/** Searchs an element with the given comparison function
		 *
		 * @param	f the comparison function used for search
		 * @return	true if found, false otherwise */
		template <class F>
		bool has(const F& f) const;

		/** Searchs an element with the given comparison function
		 *
		 * @param	f the comparison function used for search
		 * @return	the element searched, undetermined if not found */
		template <class F>
		const T& get(const F& f) const;
	private:
		/** Creates a new Repository */
		Repository() = default;
	};

}

#include "Repository.hpp"

#endif		// REPOSITORY_H
