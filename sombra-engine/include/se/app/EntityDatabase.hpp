#ifndef ENTITY_DATABASE_HPP
#define ENTITY_DATABASE_HPP

#include <memory>
#include <functional>

namespace se::app {

	/** Struct ComponentTypes, holds metadata about the ComponentTypes */
	struct EntityDatabase::ComponentTypes
	{
		/** The number of ComponentTypes */
		static std::size_t sCount;
	};


	/** Struct ComponentType, it's used for calculating the Component Id
	 * automatically of the given Component type @tparam T */
	template <typename T>
	struct EntityDatabase::ComponentType
	{
		/** @return	the Component id of @tparam T */
		static std::size_t getId()
		{
			static std::size_t sComponentId = ComponentTypes::sCount++;
			return sComponentId;
		};
	};


	/**
	 * Class IComponentTable, it's the Interface that every ComponentTable must
	 * implement
	 */
	class EntityDatabase::IComponentTable
	{
	public:
		/** Class destructor */
		virtual ~IComponentTable() = default;

		/** Check if the given Entity has a Component
		 *
		 * @param	entity the Entity that owns the Component
		 * @return	true if the Entity has the Component, false otherwise */
		virtual bool hasComponent(Entity entity) const = 0;

		/** Removes the Component owned by the given Entity
		 *
		 * @param	entity the Entity that owns the Component */
		virtual void removeComponent(Entity entity) = 0;
	};


	/**
	 * Class ITComponentTable, it's the Interface that every ComponentTable
	 * with a Component of type @tparam T must implement
	 */
	template <typename T>
	class EntityDatabase::ITComponentTable : public IComponentTable
	{
	public:		// Functions
		/** Class destructor */
		virtual ~ITComponentTable() = default;

		/** Adds a Component to the ITComponentTable and makes the given Entity
		 * its owner
		 *
		 * @param	entity the Entity that will own the Component
		 * @param	component the Component to add */
		virtual void addComponent(Entity entity, T&& component) = 0;

		/** Returns the Component of the given Entity
		 *
		 * @param	entity the Entity that owns the Component
		 * @return	a pointer to the Component of the Entity, nullptr if the
		 *			Entity doesn't own any Component */
		virtual T* getComponent(Entity entity) = 0;

		/** Returns the Entity of the given Component
		 *
		 * @param	component a pointer to the Component of the Entity
		 * @return	the Entity that owns the Component, kNullEntity if it
		 *			wasn't found */
		virtual Entity getEntity(const T* component) = 0;

		/** Removes the given Component
		 *
		 * @param	component a pointer to the Component to remove */
		virtual void removeComponent(const T* component) = 0;

		/** Iterates over all the Components of the ITComponentTable
		 *
		 * @param	callback the function to call for each Component */
		virtual void iterateComponents(std::function<void(T&)> callback) = 0;
	};


	/**
	 * Class ComponentTable, it holds all the Components of the type @tparam T
	 * and their relation with the EntityDatabase Entities
	 */
	template <typename T>
	class EntityDatabase::ComponentTable : public ITComponentTable<T>
	{
	private:	// Attributes
		/** The Components added to the ComponentTable */
		utils::PackedVector<T> mComponents;

		/** Maps each Entity with the index of its respecive Component in
		 * @see mComponents */
		std::unordered_map<Entity, std::size_t> mEntityComponentMap;

		/** Maps each Component index in @see mComponents with its respecive
		 * Entity */
		std::unordered_map<T*, Entity> mComponentEntityMap;

	public:		// Functions
		/** Creates a new ComponentTable
		 *
		 * @param	maxComponents the maximum number of Components that the
		 *			ComponentTable can hold */
		ComponentTable(std::size_t maxComponents)
		{
			mComponents.reserve(maxComponents);
			mEntityComponentMap.reserve(maxComponents);
			mComponentEntityMap.resize(maxComponents, kNullEntity);
		};

		/** Adds a Component to the ComponentTable and makes the given Entity
		 * its owner
		 *
		 * @param	entity the Entity that will own the Component
		 * @param	component the Component to add */
		virtual void addComponent(Entity entity, T&& component) override
		{
			if (mComponents.size() < mComponents.capacity()) {
				auto it = mComponents.emplace(std::move(component));
				mEntityComponentMap.emplace(entity, it.getIndex());
				mComponentEntityMap.emplace(&(*it), entity);
			}
		};

		/** Check if the given Entity has a Component
		 *
		 * @param	entity the Entity that owns the Component
		 * @return	true if the Entity has the Component, false otherwise */
		virtual bool hasComponent(Entity entity) const override
		{
			return mEntityComponentMap.find(entity) != mEntityComponentMap.end();
		};

		/** Returns the Component of the given Entity
		 *
		 * @param	entity the Entity that owns the Component
		 * @return	a pointer to the Component of the Entity, nullptr if the
		 *			Entity doesn't own any Component */
		virtual T* getComponent(Entity entity) override
		{
			auto it = mEntityComponentMap.find(entity);
			if (it != mEntityComponentMap.end()) {
				return &mComponents[it->second];
			}
			return nullptr;
		};

		/** Returns the Entity of the given Component
		 *
		 * @param	component a pointer to the Component of the Entity
		 * @return	the Entity that owns the Component, kNullEntity if it
		 *			wasn't found */
		virtual Entity getEntity(const T* component) override
		{
			auto it = mComponentEntityMap.find(component);
			if (it != mComponentEntityMap.end()) {
				return it->second;
			}
			return kNullEntity;
		};

		/** Removes the Component owned by the given Entity
		 *
		 * @param	entity the Entity that owns the Component */
		virtual void removeComponent(Entity entity) override
		{
			auto it1 = mEntityComponentMap.find(entity);
			if (it1 != mEntityComponentMap.end()) {
				auto it2 = mComponentEntityMap.find(&mComponents[it1->second]);
				mComponents.erase( mComponents.begin().setIndex(it1->second) );
				mEntityComponentMap.erase(it1);
				mComponentEntityMap.erase(it2);
			}
		};

		/** Removes the given Component
		 *
		 * @param	component a pointer to the Component to remove */
		virtual void removeComponent(const T* component) override
		{
			auto it1 = mComponentEntityMap.find(component);
			if (it1 != mComponentEntityMap.end()) {
				auto it2 = mEntityComponentMap.find(it1->second);
				mComponents.erase( mComponents.begin().setIndex(it2->second) );
				mEntityComponentMap.erase(it2);
				mComponentEntityMap.erase(it1);
			}
		};

		/** Iterates over all the Components of the ComponentTable
		 *
		 * @param	callback the function to call for each Component */
		virtual void iterateComponents(std::function<void(T&)> callback) override
		{
			for (auto& component : mComponents) {
				callback(component);
			}
		};
	};


	/**
	 * Class ComponentUPtrTable, it holds all the Components of the type
	 * @tparam T and their Derived classes and their relation with the
	 * EntityDatabase Entities
	 */
	template <typename T>
	class EntityDatabase::ComponentUPtrTable : public ITComponentTable<T>
	{
	private:	// Attributes
		/** The Components added to the ComponentUPtrTable */
		utils::PackedVector<std::unique_ptr<T>> mComponents;

		/** Maps each Entity with the index of its respecive Component in
		 * @see mComponents */
		std::unordered_map<Entity, std::size_t> mEntityComponentMap;

		/** Maps each Component index in @see mComponents with its respecive
		 * Entity */
		std::unordered_map<T*, Entity> mComponentEntityMap;

	public:		// Functions
		/** Creates a new ComponentUPtrTable
		 *
		 * @param	maxComponents the maximum number of Components that the
		 *			ComponentUPtrTable can hold */
		ComponentUPtrTable(std::size_t maxComponents)
		{
			mComponents.reserve(maxComponents);
			mEntityComponentMap.reserve(maxComponents);
			mComponentEntityMap.resize(maxComponents, kNullEntity);
		};

		/** Adds a Component to the ComponentUPtrTable and makes the given Entity
		 * its owner
		 *
		 * @param	entity the Entity that will own the Component
		 * @param	component the Component to add */
		virtual void addComponent(Entity entity, T&& component) override
		{
			if (mComponents.size() < mComponents.capacity()) {
				auto it = mComponents.emplace(std::move(component));
				mEntityComponentMap.emplace(entity, it.getIndex());
				mComponentEntityMap.emplace(&(*it), entity);
			}
		};

		/** Check if the given Entity has a Component
		 *
		 * @param	entity the Entity that owns the Component
		 * @return	true if the Entity has the Component, false otherwise */
		virtual bool hasComponent(Entity entity) const override
		{
			return mEntityComponentMap.find(entity) != mEntityComponentMap.end();
		};

		/** Returns the Component of the given Entity
		 *
		 * @param	entity the Entity that owns the Component
		 * @return	a pointer to the Component of the Entity, nullptr if the
		 *			Entity doesn't own any Component */
		virtual T* getComponent(Entity entity) override
		{
			auto it = mEntityComponentMap.find(entity);
			if (it != mEntityComponentMap.end()) {
				return mComponents[it->second].get();
			}
			return nullptr;
		};

		/** Returns the Entity of the given Component
		 *
		 * @param	component a pointer to the Component of the Entity
		 * @return	the Entity that owns the Component, kNullEntity if it
		 *			wasn't found */
		virtual Entity getEntity(const T* component) override
		{
			auto it = mComponentEntityMap.find(component);
			if (it != mComponentEntityMap.end()) {
				return it->second;
			}
			return kNullEntity;
		};

		/** Removes the Component owned by the given Entity
		 *
		 * @param	entity the Entity that owns the Component */
		virtual void removeComponent(Entity entity) override
		{
			auto it1 = mEntityComponentMap.find(entity);
			if (it1 != mEntityComponentMap.end()) {
				auto it2 = mComponentEntityMap.find(&mComponents[it1->second]);
				mComponents.erase( mComponents.begin().setIndex(it1->second) );
				mEntityComponentMap.erase(it1);
				mComponentEntityMap.erase(it2);
			}
		};

		/** Removes the given Component
		 *
		 * @param	component a pointer to the Component to remove */
		virtual void removeComponent(const T* component) override
		{
			auto it1 = mComponentEntityMap.find(component);
			if (it1 != mComponentEntityMap.end()) {
				auto it2 = mEntityComponentMap.find(it1->second);
				mComponents.erase( mComponents.begin().setIndex(it2->second) );
				mEntityComponentMap.erase(it2);
				mComponentEntityMap.erase(it1);
			}
		};

		/** Iterates over all the Components of the ComponentUPtrTable
		 *
		 * @param	callback the function to call for each Component */
		virtual void iterateComponents(std::function<void(T&)> callback) override
		{
			for (auto& component : mComponents) {
				callback(*component);
			}
		};
	};


	EntityDatabase::~EntityDatabase()
	{
		iterateEntities([this](Entity entity) {
			removeEntity(entity);
		});
	}


	template <typename T>
	void EntityDatabase::addComponentTable(std::size_t maxComponents, bool hasDerived)
	{
		auto table = hasDerived?
			std::make_unique<ComponentUPtrTable<T>>(maxComponents) :
			std::make_unique<ComponentTable<T>>(maxComponents);
		mComponentTables.emplace(ComponentType<T>::getId(), std::move(table));
	}


	void EntityDatabase::addSystem(ISystem* system)
	{
		mSystems.push_back(system);
	}


	Entity EntityDatabase::addEntity()
	{
		if (mRemovedEntities.empty()) {
			if (mLastEntity != static_cast<Entity>(mMaxEntities - 1)) {
				return ++mLastEntity;
			}
		}
		else {
			auto it = mRemovedEntities.begin();
			Entity ret = *it;
			mRemovedEntities.erase(it);

			return ret;
		}

		return kNullEntity;
	}


	template <typename F>
	void EntityDatabase::iterateEntities(F&& callback)
	{
		for (Entity entity = 1; entity <= mLastEntity; ++entity) {
			if (mRemovedEntities.find(entity) == mRemovedEntities.end()) {
				callback(entity);
			}
		}
	}


	void EntityDatabase::removeEntity(Entity entity)
	{
		for (auto& pair : mComponentTables) {
			pair.second->removeComponent(entity);
		}
		mRemovedEntities.emplace(entity);
	}


	template <typename T>
	void EntityDatabase::addComponent(Entity entity, T&& component)
	{
		return getTable<T>().addComponent(entity, std::move(component));
	}


	template <typename... Args, typename F>
	void EntityDatabase::iterateComponents(F&& callback)
	{
		iterateEntities([&](Entity entity) {
			auto components = getComponents<Args...>(entity);

			bool hasAllComponents = true;
			std::apply(
				[&hasAllComponents](auto&&... component) { ((hasAllComponents &= (component != nullptr)), ...); },
				components
			);

			if (hasAllComponents) {
				auto params = std::tuple_cat(std::make_tuple(entity), components);
				std::apply(callback, params);
			}
		});
	}


	template <typename T1, typename T2, typename... Args>
	std::tuple<T1*, T2*, Args*...> EntityDatabase::getComponents(Entity entity)
	{
		auto component = getTable<T1>().getComponent(entity);
		return std::tuple_cat(std::make_tuple(component), getComponents<T2, Args...>(entity));
	}


	template <typename T>
	std::tuple<T*> EntityDatabase::getComponents(Entity entity)
	{
		auto component = getTable<T>().getComponent(entity);
		return std::make_tuple(component);
	}


	template <typename T>
	void EntityDatabase::removeComponent(Entity entity)
	{
		return getTable<T>().removeComponent(entity);
	}


	template <typename T>
	Entity EntityDatabase::getEntity(const T* component)
	{
		return getTable<T>().getEntity(component);
	}

// Private functions
	template <typename T>
	EntityDatabase::ITComponentTable<T>& EntityDatabase::getTable()
	{
		return *dynamic_cast<ITComponentTable<T>*>( mComponentTables[ComponentType<T>::getId()].get() );
	}

}

#endif		// ENTITY_DATABASE_HPP
