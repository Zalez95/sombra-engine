#ifndef ENTITY_DATABASE_H
#define ENTITY_DATABASE_H

#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "../utils/PackedVector.h"
#include "Entity2.h"
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
		struct ComponentTypes;
		template <typename T> struct ComponentType;
		class IComponentTable;
		template <typename T> class ITComponentTable;
		template <typename T> class ComponentTable;
		template <typename T> class ComponentUPtrTable;
		using IComponentTableUPtr = std::unique_ptr<IComponentTable>;

	private:	// Attributes
		/** The maximum number of Entities that the EntityDatabase can hold */
		std::size_t mMaxEntities;

		/** The entities removed from the EntityDatabase. This entities are
		 * stored here so they can be reused later */
		std::unordered_set<Entity> mRemovedEntities;

		/** The Entity with the highest identifier */
		Entity mLastEntity;

		/** Maps each Component type id with its respective ComponentTable */
		std::unordered_map<std::size_t, IComponentTableUPtr> mComponentTables;

		/** The ISystems to notify of new Entities or Components */
		std::vector<ISystem*> mSystems;

	public:		// Functions
		/** Creates a new EntityDatabase
		 * @param	maxEntities the maximum number of Entities that the
		 *			EntityDatabase can hold */
		EntityDatabase(std::size_t maxEntities) :
			mMaxEntities(maxEntities), mLastEntity(kNullEntity) {};

		/** Class destructor */
		~EntityDatabase();

		/** Adds a new ComponentTable to the EntityDatabase so the Components
		 * with the type @tparam T can be added to the Entities
		 *
		 * @param	maxComponents the maximum number of Components of that type
		 *			that can be stored in the table
		 * @param	hasDerived if there are going to be stored derived types
		 *			of @tparam T in the table or not
		 * @note	this function must be called for each Component type
		 *			before using any of the other functions */
		template <typename T>
		void addComponentTable(
			std::size_t maxComponents, bool hasDerived = false
		);

		/** Adds the given System so it can be notified of new Entities and
		 * Components
		 *
		 * @param	system a pointer to the System to add */
		void addSystem(ISystem* system);

		/** Creates a new Entity
		 *
		 * @return	the new Entity, @see kNullEntity if it couldn't be
		 *			created */
		Entity addEntity();

		/** Iterates all the Entities added to the EntityDatabase
		 *
		 * @param	callback the callback function to call for each Entity */
		template <typename F>
		void iterateEntities(F&& callback);

		/** Removes the given Entity
		 *
		 * @param	entity the Entity to remove */
		void removeEntity(Entity entity);

		/** Adds a Component with @tparam T to the given Entity
		 *
		 * @param	entity the Entity that will own the Component
		 * @param	component the Component to add */
		template <typename T>
		void addComponent(Entity entity, T&& component);

		/** Iterates all the Entities that have all of the requested Components
		 * calling the given callback function
		 *
		 * @param	callback the callback function to call for each Entity */
		template <typename... Args, typename F>
		void iterateComponents(F&& callback);

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

		/** Removes the Component with @tparam T from the given Entity
		 *
		 * @param	entity the Entity that owns the Component */
		template <typename T>
		void removeComponent(Entity entity);

		/** Returns the Entity that owns the given Component
		 *
		 * @param	component a pointer to the Component
		 * @return	the Entity that owns the Component, kNullEntity if it
		 *			wasn't found */
		template <typename T>
		Entity getEntity(const T* component);
	private:
		/** @return	a reference to the ITComponentTable of the Component with
		 *			type @tparam T */
		template <typename T>
		ITComponentTable<T>& getTable();
	};

}

#include "EntityDatabase.hpp"

#endif		// ENTITY_DATABASE_H
