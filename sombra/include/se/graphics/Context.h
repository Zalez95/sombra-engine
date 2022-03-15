#ifndef GRAPHICS_CONTEXT_H
#define GRAPHICS_CONTEXT_H

#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include "se/utils/PackedVector.h"
#include "core/Bindable.h"

namespace se::graphics {

	/**
	 * Class Context. It holds all the Bindables of a graphics context.
	 * It can be used for performing graphics operation in a deferred way from
	 * the main thread (in a thread safe way). Each Bindable object in the
	 * Context has asociated a reference count so they can be
	 * referenced from multiple places and be automatically destroyed if they
	 * don't have any users.
	 */
	class Context
	{
	public:		// Nested types
		class BindableRef;
		template <typename T> class TBindableRef;
		class Query;
	private:
		/** Struct BindableResource, holds the Bindable with its metadata */
		struct BindableResource
		{
			/** A pointer to the Bindable stored */
			std::unique_ptr<Bindable> bindable;

			/** The metadata of the bindable, the first bit tells if the
			 * bindable should be destroyed or not, the following 7 bits tells
			 * the type id of the Bindable, and the rest of the bits stores
			 * the number of users of the Bindable */
			std::uint32_t metadata = 0;
		};

	private:	// Attributes
		/** The number of Bindable types */
		static uint32_t sBindableTypeCount;

		/** All the bindables of the Context */
		utils::PackedVector<BindableResource> mBindables;

		/** The Command Queue (FIFO) used for interacting with the Graphics API
		 * or @see mBindables */
		std::vector<std::function<void()>> mCommandQueue;

		/** The mutex used for protecting @see mCommandQueue */
		std::recursive_mutex mCommandMutex;

		/** The condition variable used with @see mCommandMutex for protecting
		 * waiting */
		std::condition_variable_any mCommandCV;

	public:		// Functions
		/** Creates a new Context */
		Context() = default;
		Context(const Context& other) = delete;
		Context(Context&& other) = default;

		/** Class destructor
		 * @note	this function must be executed from the main graphics
		 *			thread */
		~Context();

		/** Assignment operator */
		Context& operator=(const Context& other) = delete;
		Context& operator=(Context&& other) = default;

		/** Executes all the Commands sent to the Context in a FIFO order
		 *
		 * @return	a reference to the current Context object
		 * @note	this function must be executed from the main graphics
		 *			thread */
		Context& update();

		/** Waits until all the Commands submitted to the Context have
		 * finished
		 *
		 * @return	a reference to the current Context object
		 * @note	you must not wait inside of a Command */
		Context& wait();

		/** Executes the given command
		 *
		 * @param	command the callback function to execute from the main
		 *			thread
		 * @return	a reference to the current Context object */
		Context& execute(const std::function<void(Query&)>& command);

		/** Creates a new Bindable of type @tparam T
		 *
		 * @param	args the Arguments used for creating the Bindable
		 * @return	a reference to the new Bindable, it will be invalid in case
		 *			of error */
		template <typename T, typename... Args>
		TBindableRef<T> create(Args&&... args);

		/** Inserts a Bindable into the Context
		 *
		 * @param	bindable a pointer to the Bindable to insert
		 * @return	a reference to the new Bindable, it will be invalid in case
		 *			of error
		 * @note	this bindable already created will be removed from the
		 *			Context if it has no users, but the object won't be
		 *			destroyed */
		template <typename T>
		TBindableRef<T> insert(T* bindable);
	private:
		/** Creates a new Bindable by clonning the given one
		 *
		 * @param	index the index of the Bindable to clone
		 * @return	a reference to the new cloned Bindable. If the BindableRef
		 *			returned is invalid that means that the original object
		 *			no longer existed and it wasn't clonned */
		BindableRef clone(std::size_t index);

		/** Checks if the type of the Bindable located at the given index is the
		 * same than @tparam T
		 *
		 * @param	index the index of the Bindable to check
		 * @return	true if the type is the same than @tparam T, false
		 *			otherwise */
		template <typename T>
		bool checkType(std::size_t index);

		/** Adds a user to the Bindable located at the given index
		 *
		 * @param	index the index of the Bindable to add a user */
		void addUser(std::size_t index);

		/** Removes a user from the Bindable located at the given index
		 *
		 * @param	index the index of the Bindable to remove a user
		 * @note	if an element has zero elements it will be removed from the
		 *			Context */
		void removeUser(std::size_t index);

		/** Removes the Bindable located at the given index, destroying it if
		 * it has the destroy flag enabled
		 *
		 * @param	index the index of the Bindable to remove */
		void remove(std::size_t index);

		/** @return	the type id of the @tparam T Bindable type */
		template <typename T>
		static uint32_t getBindableTypeId();
	};


	/** Class BindableRef, it's a reference to a Bindable of the
	 * Context */
	class Context::BindableRef
	{
	public:		// Nested types
		/** Struct HashFunc, used for calculating the hash value of
		 * BindableRefs */
		struct HashFunc
		{
			/** Calculates the hash value of the given BindableRef
			 *
			 * @param	ref the BindableRef to calculate its hash value
			 * @return	the hash value */
			std::size_t operator()(const BindableRef& ref) const;
		};

	protected:	// Attributes
		/** The Context that holds the bindable referenced */
		Context* mParent;

		/** The index to the bindable referenced in the Context */
		std::size_t mIndex;

	public:		// Functions
		/** Creates a new BindableRef
		 *
		 * @param	parent a pointer to the Context that holds the
		 *			Bindable referenced
		 * @param	index the index to the Bindable referenced in the
		 *			Context */
		BindableRef(Context* parent = nullptr, std::size_t index = 0);
		BindableRef(const BindableRef& other);
		BindableRef(BindableRef&& other);

		/** Class destructor */
		virtual ~BindableRef();

		/** Assignment operator */
		BindableRef& operator=(const BindableRef& other);
		BindableRef& operator=(BindableRef&& other);

		/** @return	true if the BindableRef reference to any element,
		 *			false otherwise */
		operator bool() const { return mParent; };

		/** @return	the parent Context of the BindableRef */
		Context* getParent() const { return mParent; };

		/** @return	the index of the Bindable of the BindableRef */
		std::size_t getIndex() const { return mIndex; };

		/** Compares the given BindableRefs
		 *
		 * @param	ref1 the first BindableRef to compare
		 * @param	ref2 the second BindableRef to compare
		 * @return	true if both BindableRefs are equal, false otherwise */
		friend bool operator==(
			const BindableRef& ref1, const BindableRef& ref2
		) { return (ref1.mParent == ref2.mParent)
				&& (ref1.mIndex == ref2.mIndex); };

		/** Compares the given BindableRefs
		 *
		 * @param	ref1 the first BindableRef to compare
		 * @param	ref2 the second BindableRef to compare
		 * @return	true if both BindableRefs are different, false
		 *			otherwise */
		friend bool operator!=(
			const BindableRef& ref1, const BindableRef& ref2
		) { return !(ref1 == ref2); };

		/** @return	a BindableRef to a clone of the Bindable pointed
		 *			by the current one */
		BindableRef clone() const
		{ return mParent->clone(mIndex); };
	};


	/** Class BindableRef, it's a reference to a Bindable of type @tparam T
	 * of the Context */
	template <typename T>
	class Context::TBindableRef : public Context::BindableRef
	{
	public:		// Functions
		/** Creates a new BindableRef
		 *
		 * @param	parent a pointer to the Context that holds the
		 *			Bindable referenced
		 * @param	index the index to the Bindable referenced in the
		 *			Context
		 * @note	if the type @tparam T doesn't match the type of the
		 *			pointed Bindable, the TBindableRef won't be valid.
		 *			Inheritance is not supported */
		TBindableRef(Context* parent = nullptr, std::size_t index = 0);
		TBindableRef(const TBindableRef& other) = default;
		TBindableRef(TBindableRef&& other) = default;

		/** Class destructor */
		virtual ~TBindableRef() = default;

		/** Assignment operator */
		TBindableRef& operator=(const TBindableRef& other) = default;
		TBindableRef& operator=(TBindableRef&& other) = default;

		/** Creates a TBindableRef from the given BindableRef
		 *
		 * @param	other the BindableReffrom which the TBindableRef will
		 *			be created
		 * @return	the new TBindableRef
		 * @note	if the type @tparam T doesn't match the type of the
		 *			pointed Bindable, the TBindableRef won't be valid */
		static TBindableRef from(const BindableRef& other)
		{ return TBindableRef(other.getParent(), other.getIndex()); };

		/** Updates the Bindable of type @tparam T referenced by the
		 * BindableRef
		 *
		 * @param	callback the callback function used for updating the
		 *			Bindable, it must accept a reference to a Bindable
		 *			object of type T
		 * @return	a reference to the current TBindableRef object */
		template <typename F>
		TBindableRef edit(F&& callback) const;

		/** Updates the Bindable of type @tparam T referenced by the
		 * BindableRef
		 *
		 * @param	callback the callback function used for updating the
		 *			Bindable, it must accept a reference to a Query object and a
		 *			Bindable object of type T
		 * @return	a reference to the current TBindableRef object */
		template <typename F>
		TBindableRef qedit(F&& callback) const;
	};


	/** Class Query, it's the object used for accessing to the Bindables
	 * with the BindableRefs */
	class Context::Query
	{
	private:	// Attributes
		/** The Graphics Context */
		Context& mParent;

	public:		// Functions
		/** Create a new Query object
		 *
		 * @param	parent the parent Graphics Context of the Query */
		Query(Context& parent) : mParent(parent) {};

		/** Returns a pointer to the Bindable object pointed by the given
		 * BindableRef
		 *
		 * @param	ref the BindableRef object that points to the Bindable
		 * @return	an actual pointer to the Bindable object, nullptr if it
		 *			doesn't exist */
		Bindable* getBindable(const BindableRef& ref) const;

		/** Returns a pointer to the Bindable object pointed by the given
		 * BindableRef
		 *
		 * @param	ref the BindableRef object that points to the Bindable
		 * @return	an actual pointer to the Bindable object, nullptr if it
		 *			doesn't exist */
		template <typename T>
		T* getTBindable(const TBindableRef<T>& ref) const
		{ return static_cast<T*>(getBindable(ref)); }
	};

}

#include "Context.hpp"

#endif		// GRAPHICS_CONTEXT_H
