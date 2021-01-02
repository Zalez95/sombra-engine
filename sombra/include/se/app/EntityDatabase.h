#ifndef ENTITY_DATABASE_H
#define ENTITY_DATABASE_H

#include <memory>
#include <vector>
#include <unordered_set>
#include "../utils/PackedVector.h"
#include "Entity.h"
#include "ISystem.h"

namespace se::app {

	/**
	 * Class EntityDatabase, it holds all the Entities and their respective
	 * Components stored like a Database. It is used implementing an
	 * Entity-Component System in which the Entities are nothing more than an
	 * identifier and the Database is used for accessing to their Components
	 */
	class EntityDatabase
	{
	private:	// Nested types
		class IComponentTable;
		template <typename T> class ITComponentTable;
		template <typename T> class ComponentTable;
		template <typename T> class ComponentTableUPtr;
		using IComponentTableUPtr = std::unique_ptr<IComponentTable>;
	public:
		/** Class ComponentMask, it holds the bit mask with a state for each
		 * Component type */
		class ComponentMask
		{
		private:	// Attributes
			/** The bit state for each Component */
			std::vector<bool> mBitMask;

		public:		// Functions
			/** Creates a new ComponentMask
			 *
			 * @param	value the initial value of all the bits */
			ComponentMask(bool value = false);

			/** Returns the position of the bitmask located at the given index
			 *
			 * @param	index the position to check
			 * @return	the value at the given position */
			bool operator[](std::size_t index) const
			{ return mBitMask[index]; };

			/** Sets the value for the given @tparam T Component
			 *
			 * @param	value the new bit value */
			template <typename T>
			ComponentMask& set(bool value = true);

			/** @return	the value for the given @tparam T Component */
			template <typename T>
			bool get() const;
		};

	private:	// Attributes
		/** The number of different Component types */
		static std::size_t sComponentTypeCount;

		/** The maximum number of Entities that the EntityDatabase can hold */
		std::size_t mMaxEntities;

		/** The Entity with the highest identifier */
		Entity mLastEntity;

		/** The entities removed from the EntityDatabase. This entities are
		 * stored here so they can be reused later */
		std::unordered_set<Entity> mRemovedEntities;

		/** The Components active for each Entity */
		std::vector<bool> mActiveComponents;

		/** All the ComponentTables added to the EntityDatabase indexed by their
		 * Component type Id */
		std::vector<IComponentTableUPtr> mComponentTables;

		/** The ISystems to notify of new Entities or Components */
		std::vector<std::pair<ISystem*, ComponentMask>> mSystems;

	public:		// Functions
		/** Creates a new EntityDatabase
		 * @param	maxEntities the maximum number of Entities that the
		 *			EntityDatabase can hold */
		EntityDatabase(std::size_t maxEntities);

		/** Class destructor */
		~EntityDatabase();

		/** Adds a new ComponentTable to the EntityDatabase so the Components
		 * with the type @tparam T can be added to the Entities. If
		 * @tparam hasDerived is true, the derived types of @tparam T can also
		 * be added to the same table
		 *
		 * @param	maxComponents the maximum number of Components of that type
		 *			that can be stored in the table
		 * @note	this function must be called for each Component type
		 *			before using any other functions */
		template <typename T, bool hasDerived = false>
		void addComponentTable(std::size_t maxComponents);

		/** Adds the given System so it can be notified of new Entities and
		 * Components
		 *
		 * @param	system a pointer to the System to add
		 * @param	mask the changes in Components that must be notified to the
		 *			ISystem
		 * @note	if the system has been added it will just update the mask */
		void addSystem(ISystem* system, ComponentMask mask);

		/** Returns the ComponentMask of the given ISystem
		 *
		 * @param	system a pointer to the ISystem to check
		 * @return	the ComponentMask */
		ComponentMask getSystemMask(ISystem* system) const;

		/** Removes the given System so it won't longer be notified of new
		 * Entities and Components
		 *
		 * @param	system a pointer to the System to removed */
		void removeSystem(ISystem* system);

		/** Creates a new Entity
		 *
		 * @return	the new Entity, @see kNullEntity if it couldn't be
		 *			created */
		Entity addEntity();

		/** Returns the Entity that owns the given Component
		 *
		 * @param	component a pointer to the Component
		 * @return	the Entity that owns the Component, kNullEntity if it
		 *			wasn't found */
		template <typename T>
		Entity getEntity(const T* component);

		/** @return	the maximum number of Entities that can be stored in the
		 *			EntityDatabase */
		std::size_t getMaxEntities() const { return mMaxEntities; };

		/** Iterates all the Entities added to the EntityDatabase
		 *
		 * @param	callback the callback function to call for each Entity */
		template <typename F>
		void iterateEntities(F&& callback);

		/** Removes the given Entity
		 *
		 * @param	entity the Entity to remove */
		void removeEntity(Entity entity);

		/** @return	the maximum number of Components with @tparam T allowed */
		template <typename T>
		std::size_t getMaxComponents() const;

		/** Adds a Component with type @tparam T to the given Entity
		 *
		 * @param	entity the Entity that will own the Component
		 * @param	args the arguments needed for calling the constructor of
		 *			the new Element
		 * @return	a pointer to tha Component if it was added successfully,
		 *			nullptr otherwise */
		template <typename T, typename... Args>
		T* emplaceComponent(Entity entity, Args&&... args);

		/** Adds a Component with type @tparam T to the given Entity
		 *
		 * @param	entity the Entity that will own the Component
		 * @param	component the Component to add
		 * @return	a pointer to tha Component if it was added successfully,
		 *			nullptr otherwise */
		template <typename T>
		T* addComponent(Entity entity, T&& component);

		/** Adds a Component with type @tparam T to the given Entity
		 *
		 * @param	entity the Entity that will own the Component
		 * @param	component a pointer to the Component to add
		 * @return	a pointer to tha Component if it was added successfully,
		 *			nullptr otherwise */
		template <typename T>
		T* addComponent(Entity entity, std::unique_ptr<T> component);

		/** Checks if an Entity has all the given Components
		 *
		 * @param	entity the Entity that owns the Components
		 * @return	true if the Entity has all the Components, false
		 *			otherwise */
		template <typename T1, typename T2, typename... Args>
		bool hasComponents(Entity entity);

		/** Checks if an Entity has a Component with type @tparam T
		 *
		 * @param	entity the Entity that owns the Component
		 * @return	true if the Entity has the Component, false otherwise */
		template <typename T>
		bool hasComponents(Entity entity);

		/** Returns all the Components with the given types owned by the given
		 * Entity
		 *
		 * @param	entity the Entity that owns the Components
		 * @return	a tuple with the pointers to the Components in the
		 *			requested order. If any of the Components is null it's
		 *			because the Entity doesn't have any Component with
		 *			that type */
		template <typename T1, typename T2, typename... Args>
		std::tuple<T1*, T2*, Args*...> getComponents(Entity entity);

		/** Returns the Component with type @tparam T owned by the given
		 * Entity
		 *
		 * @param	entity the Entity that owns the Component
		 * @return	a tuple with a pointer to the Component. If the Component
		 *			is null then the Entity doesn't have any Component with the
		 *			given type */
		template <typename T>
		std::tuple<T*> getComponents(Entity entity);

		/** Iterates all the Entities that have all of the requested Components
		 * calling the given callback function
		 *
		 * @param	callback the callback function to call for each Entity */
		template <typename... Args, typename F>
		void iterateComponents(F&& callback);

		/** Removes the Component with @tparam T from the given Entity
		 *
		 * @param	entity the Entity that owns the Component */
		template <typename T>
		void removeComponent(Entity entity);
	private:
		/** @return	the Component id of @tparam T */
		template <typename T>
		static std::size_t getComponentTypeId();

		/** @return	a reference to the ComponentTable of the Component with
		 *			type @tparam T */
		template <typename T>
		ITComponentTable<T>& getTable() const;
	};

}

#include "EntityDatabase.hpp"

#endif		// ENTITY_DATABASE_H
