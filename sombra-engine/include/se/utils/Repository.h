#ifndef REPOSITORY_H
#define REPOSITORY_H

#include <vector>
#include <functional>

namespace se::utils {

	/**
	 * Class Repository, it holds all the elements of the given type loaded by
	 * the Application and provides a single point for accessing to them
	 */
	template <typename T, typename SizeType = unsigned long>
	class Repository
	{
	public:		// Nested types
		using SearchCallback = std::function<bool(const T&)>;

		/**
		 * Class Reference, it's used for accesing to the Elements stored in
		 * the Repository. If the number of Reference objects pointing to one
		 * Element reaches 0, it will get removed from the Repository
		 */
		class Reference
		{
		private:	// Attributes
			/** A pointer to the Repository where the Elements are stored */
			Repository* mRepository;

			/** The index of the Element that the Reference is pointing to */
			SizeType mIndex;

		public:		// Functions
			/** Creates a new Reference
			 *
			 * @param	repository a pointer to the Repository where the
			 *			Elements are stored */
			Reference(Repository* repository = nullptr);

			/** Creates a new Reference
			 *
			 * @param	repository a pointer to the Repository where the
			 *			Elements are stored
			 * @param	index the index of the Element that the Reference will
			 *			point to */
			Reference(Repository* repository, SizeType index);
			Reference(const Reference& other);
			Reference(Reference&& other);

			/** Class destructor */
			~Reference();

			/** Assignment operator */
			Reference& operator=(const Reference& other);
			Reference& operator=(Reference&& other);

			/** Casts the current Reference to a bool. True if the Reference
			 * points to an Elements still active, false otherwise */
			operator bool() const
			{ return mRepository && (mIndex < mRepository->mMaxElements); };

			/** @return	a reference to the current Element that the Reference is
			 *			pointing to */
			T& operator*() const { return mRepository->mData[mIndex]; };

			/** @return	a pointer to the current Element that the Reference is
			 *			pointing to */
			T* operator->() const { return &mRepository->mData[mIndex]; };

			/** Compares the given References
			 *
			 * @param	lhs the first References to compare
			 * @param	rhs the second References to compare
			 * @return	true if the both References are the same, false
			 *			otherwise */
			friend bool operator==(const Reference& lhs, const Reference& rhs)
			{ return (lhs.mRepository == rhs.mRepository)
				&& (lhs.mIndex == rhs.mIndex); };

			/** Compares the given References
			 *
			 * @param	lhs the first References to compare
			 * @param	rhs the second References to compare
			 * @return	true if the both References are different, false
			 *			otherwise */
			friend bool operator!=(const Reference& lhs, const Reference& rhs)
			{ return !(lhs == rhs); };

			/** Compares the given References
			 *
			 * @param	lhs the first References to compare
			 * @param	rhs the second References to compare
			 * @return	true if lhs has a smaller value than rhs, false
			 *			otherwise */
			friend bool operator<(const Reference& lhs, const Reference& rhs)
			{ return lhs.mIndex < rhs.mIndex; };

			/** @return	the index of the Element that the Reference is pointing
			 *			to in the Repository */
			SizeType get() const { return mIndex; };
		};

	private:	// Attributes
		/** The maximum number of Elements that can be sotred in the
		 * Repository */
		SizeType mMaxElements;

		/** All the data stored in the Repository */
		std::vector<T> mData;

		/** All the data stored in the Repository */
		std::vector<unsigned long> mRefCounts;

	public:
		/** Creates a new Repository
		 *
		 * @param	maxElements the maximum number of Elements that can be
		 *			stored in the Repository */
		Repository(SizeType maxElements);

		/** Creates a new element in the Repository
		 *
		 * @param	args the argument list with which we are going to create
		 *			the new Element
		 * @return	a Reference object to the new Element */
		template <typename... Args>
		Reference add(Args&&... args);

		/** Searchs an element with the given comparison function
		 *
		 * @param	callback the comparison function used for searching
		 * @return	a Reference object to the Element searched */
		Reference find(const SearchCallback& callback);

		/** Increments the number of references pointing to the Element located
		 * at the given index
		 *
		 * @param	index the index of the Element */
		void incrementRefCount(SizeType index);

		/** Decrements the number of references pointing to the Element located
		 * at the given index
		 *
		 * @param	index the index of the Element */
		void decrementRefCount(SizeType index);
	};

}

#include "Repository.hpp"

#endif		// REPOSITORY_H
