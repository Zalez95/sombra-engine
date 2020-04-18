#ifndef COMPONENT_DATABASE_SYSTEM_H
#define COMPONENT_DATABASE_SYSTEM_H

#include <tuple>
#include <bitset>
#include <vector>
#include <functional>

namespace se::app {

	/**
	 * Class ComponentDatabase. It holds all the Components of the Entities
	 */
	template <typename SizeType = unsigned long, typename... Types>
	class ComponentDatabase
	{
	public:		// Nested types
		using EntityId = SizeType;
		using size_type = SizeType;
		using ActiveColumns = std::bitset<sizeof...(Types)>;
		using EntityCallback = std::function<void(EntityId)>;

	private:	// Attributes
		/** The maximum number of entities allowed in the ComponentDatabase */
		SizeType mMaxEntities;

		/** Stores which Entities are active or has been released. The Entities
		 * aren't actually removed from the Database, they are released for
		 * later usage. This way the pointers and indexes to the Entities don't
		 * get invalidated */
		std::vector<bool> mActiveEntities;

		/** A vector of bitsets that tells which of the Components are active
		 * for each of the Entities */
		std::vector<ActiveColumns> mActiveComponents;

		/** The components of the Entities */
		std::tuple< std::vector<Types>... > mComponents;

	public:		// Functions
		/** Creates a new ComponentDatabase
		 *
		 * @param	maxEntities the maximum number of Entities allowed in the
		 *			Database */
		ComponentDatabase(SizeType maxEntities);

		/** Class destructor */
		virtual ComponentDatabase() = default;

		/** @return	the maximum number of entities allowed in the
		 *			ComponentDatabase */
		SizeType getMaxEntities() const { return mMaxEntities; };

		/** Adds a new Entity to the ComponentDatabase
		 *
		 * @return	The EntityId of the new Entity, mMaxEntities if it couldn't
		 *			be added */
		EntityId addEntity();

		/** Removes the given Entity from the ComponentDatabase
		 *
		 * @return	entityId the EntityId of the Entity to remove */
		void removeEntity(EntityId entityId);

		/** Returns the requested Component of the given Entity
		 *
		 * @param	entityId the id of the Entity whose Component we want to get
		 * @return	a pointer to the requested Component, nullptr if it wasn't
		 *			found */
		template <typename T, unsigned long componentId>
		T* getComponent(EntityId entityId);

		/** Returns the requested Component of the given Entity
		 *
		 * @param	entityId the id of the Entity whose Component we want to get
		 * @return	a pointer to the requested Component, nullptr if it wasn't
		 *			found */
		template <typename T, unsigned long componentId>
		const T* getComponent(EntityId entityId) const;

		/** Adds a Component to the given Entity
		 *
		 * @param	entityId the id of the Entity to add the Component
		 * @return	args the constructor arguments for creating the Component */
		template <typename T, unsigned long componentId, typename... Args>
		void addComponent(EntityId entityId, Args&&... args);

		/** Removes the given Component from the given Entity
		 *
		 * @param	entityId the id of the Entity which we want to remove the
		 *			Component */
		template <unsigned long componentId>
		void removeComponent(EntityId entityId);

		/** Function used for iterating the Entities added to the
		 * ComponentDatabase
		 *
		 * @param	callback the function to call for each Entity
		 * @param	filters the Components that the Entity must have so it
		 *			doesn't get omitted while iterating, all by default */
		void processEntities(
			const EntityCallback& callback,
			ActiveColumns filters = ActiveColumns().set()
		) const;
	};

}

#include "ComponentDatabase.hpp"

#endif		// COMPONENT_DATABASE_SYSTEM_H
