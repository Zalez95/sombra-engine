#ifndef ECS_H
#define ECS_H

#include <memory>
#include <vector>
#include <unordered_set>
#include "../utils/PackedVector.h"
#include "Entity.h"
#include "events/EventManager.h"

namespace se::app {

	class ISystem;


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

			/** Returns the value of the bitmask located at the given index
			 *
			 * @param	index the position to check
			 * @return	the value at the given position */
			bool operator[](std::size_t index) const
			{ return mBitMask[index]; };

			/** Sets the value of the bitmask located at the given index
			 *
			 * @param	index the position to set
			 * @param	value the new bit value */
			ComponentMask& set(std::size_t index, bool value = true)
			{ mBitMask[index] = value; return *this; };

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
		 *			created
		 * @note	the Entity will have no Components, but all of them will be
		 *			enabled by default */
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
		 * @param	onlyEnabled true if we only want to get the Components
		 *			that are enabled, false if we want to get all the
		 *			Components wether they are enabled or not
		 * @return	a tuple with the pointers to the Components in the
		 *			requested order. If the Entity doesn't have any of the
		 *			Components or it's disabled and @see onlyEnabled is true
		 *			it will return nullptr in its place */
		template <typename T1, typename T2, typename... Args>
		std::tuple<T1*, T2*, Args*...>
			getComponents(Entity entity, bool onlyEnabled = false);

		/** Returns the Component with type @tparam T owned by the given
		 * Entity
		 *
		 * @param	entity the Entity that owns the Component
		 * @param	onlyEnabled true if we only want to get the Component
		 *			if it's enabled, false if we want to get the Component
		 *			wether it's enabled or not
		 * @return	a tuple with a pointer to the Component. If the Entity
		 *			doesn't have the Component or it's disabled and
		 *			@see onlyEnabled is true it will return nullptr in its
		 *			place */
		template <typename T>
		std::tuple<T*> getComponents(Entity entity, bool onlyEnabled = false);

		/** Iterates all the Entities that have all of the requested Components
		 * calling the given callback function
		 *
		 * @param	callback the callback function to call for each Entity
		 * @param	onlyEnabled true if we only want to iterate the Entities
		 *			with the given Components enabled, false if we want to
		 *			iterate all the Entities with the given Components wether
		 *			they are enabled or not */
		template <typename... Args, typename F>
		void iterateComponents(F&& callback, bool onlyEnabled = false);

		/** Removes the Component with @tparam T from the given Entity
		 *
		 * @param	entity the Entity that owns the Component */
		template <typename T>
		void removeComponent(Entity entity);

		/** Enables the Component with @tparam T for the given Entity so the
		 * Systems can start using it
		 *
		 * @param	entity the Entity that owns/will own the Component
		 * @note	if there is no Component currently added to the Entity, the
		 *			next time a Component is added the Systems will be notified
		 *			of it */
		template <typename T>
		void enableComponent(Entity entity);

		/** Checks if an Entity has all the given Components enabled
		 *
		 * @param	entity the Entity that owns/will own the Components
		 * @return	true if the Entity has all the Components enabled, false
		 *			otherwise */
		template <typename T1, typename T2, typename... Args>
		bool hasComponentsEnabled(Entity entity);

		/** Checks if an Entity has a Component enabled with type @tparam T
		 *
		 * @param	entity the Entity that owns/will own the Component
		 * @return	true if the Entity has the Component enabled, false
		 *			otherwise */
		template <typename T>
		bool hasComponentsEnabled(Entity entity);

		/** Disables the Component with @tparam T for the given Entity so the
		 * Systems will no longer use it
		 *
		 * @param	entity the Entity that owns/will own the Component
		 * @note	if there is no Component currently added to the Entity, the
		 *			next time a Component is added to the Entity the Systems
		 *			won't be notified of it until @see enableComponent is
		 *			called */
		template <typename T>
		void disableComponent(Entity entity);
	private:
		/** @return	the Component id of @tparam T */
		template <typename T>
		static std::size_t getComponentTypeId();

		/** @return	a reference to the ComponentTable of the Component with
		 *			type @tparam T */
		template <typename T>
		ITComponentTable<T>& getTable() const;
	};




	/**
	 * Class System, it's the interface that each System must implement. A
	 * System is used for updating the Entities Components at every clock tick.
	 * Also, it can listen for Events
	 */
	class ISystem : public IEventListener
	{
	protected:	// Attributes
		/** The EntityDatabase that holds all the Entities and their
		 * Components */
		EntityDatabase& mEntityDatabase;

		/** The elapsed time since the last @see update call */
		float mDeltaTime;

	public:		// Functions
		/** Creates a new ISystem
		 *
		 * @param	entityDatabase the EntityDatabase that holds all the
		 *			Entities */
		ISystem(EntityDatabase& entityDatabase) :
			mEntityDatabase(entityDatabase), mDeltaTime(0.0f) {};

		/** Class destructor */
		virtual ~ISystem() = default;

		/** @copydoc IEventListener::notify(const IEvent&) */
		virtual bool notify(const IEvent& /*event*/) override { return false; };

		/** Function that the EntityDatabase will call when an Entity Component
		 * is added
		 *
		 * @param	entity the Entity that holds the Component
		 * @param	mask the ComponentMask that is used for knowing which
		 *			Component has been added */
		virtual void onNewComponent(
			Entity /*entity*/, const EntityDatabase::ComponentMask& /*mask*/
		) {};

		/** Function that the EntityDatabase will call when an Entity Component
		 * is going to be removed
		 *
		 * @param	entity the Entity that holds the Component
		 * @param	mask the ComponentMask that is used for knowing which
		 *			Component is going to be removed */
		virtual void onRemoveComponent(
			Entity /*entity*/, const EntityDatabase::ComponentMask& /*mask*/
		) {};

		/** Sets the delta time of the ISystem
		 *
		 * @param	deltaTime the elapsed time since the last @see update
		 *			call */
		void setDeltaTime(float deltaTime) { mDeltaTime = deltaTime; };

		/** Function called every clock tick */
		virtual void update() {};
	protected:
		/** Tries to call the given component handler function with the correct
		 * Component type
		 *
		 * @param	componentHandler a pointer to the function to call
		 * @param	entity the Entity that holds the Component
		 * @param	mask the ComponentMask that holds the type of the Component
		 *			to check */
		template <typename S, typename C>
		void tryCallC(
			void(S::*componentHandler)(Entity, C*),
			Entity entity, const EntityDatabase::ComponentMask& mask
		);
	};

}

#include "ECS.hpp"

#endif		// ECS_H
