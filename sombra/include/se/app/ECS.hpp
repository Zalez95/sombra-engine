#ifndef ECS_HPP
#define ECS_HPP

#include <cassert>
#include <unordered_map>

namespace se::app {

	/**
	 * Class IComponentTable, it's the Interface that every ComponentTable must
	 * implement
	 */
	class EntityDatabase::IComponentTable
	{
	public:
		/** Class destructor */
		virtual ~IComponentTable() = default;

		/** @return	the maximum number of components allowed */
		virtual std::size_t getMaxComponents() const = 0;

		/** Copies a Component from the source Entity to the destination Entity
		 *
		 * @param	source the Entity that owns the Component to copy
		 * @param	destination the Entity that will own the copied Component
		 * @return	true if it was copied successfully, false otherwise */
		virtual bool copyComponent(Entity source, Entity destination) = 0;

		/** Check if the given Entity has a Component
		 *
		 * @param	entity the Entity that owns the Component
		 * @return	true if the Entity has the Component, false otherwise */
		virtual bool hasComponent(Entity entity) const = 0;

		/** Removes the Component owned by the given Entity
		 *
		 * @param	entity the Entity that owns the Component */
		virtual void removeComponent(Entity entity) = 0;

		/** Enables the Component of the given Entity
		 *
		 * @param	entity the Entity that owns the Component */
		virtual void enableComponent(Entity entity) = 0;

		/** Check if the given Entity has the Component enabled or not
		 *
		 * @param	entity the Entity that owns the Component
		 * @return	true if the Entity has the Component enabled, false
		 *			otherwise */
		virtual bool hasComponentEnabled(Entity entity) const = 0;

		/** Disables the Component of the given Entity
		 *
		 * @param	entity the Entity that owns the Component */
		virtual void disableComponent(Entity entity) = 0;
	};


	/**
	 * Class ITComponentTable, it's the Interface that every ComponentTable must
	 * implement
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
		 * @param	component the Component to add
		 * @return	a pointer to tha Component if it was added successfully,
		 *			nullptr otherwise
		 * @note	the Component will be enabled */
		virtual T* addComponent(Entity entity, T&& component) = 0;

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

		/** Iterates over all the Components of the ITComponentTable
		 *
		 * @param	callback the function to call for each Component
		 * @param	onlyEnabled true if we only want to iterate the Components
		 *			enabled, false if we want to iterate all the Components
		 *			wether they are enabled or not */
		virtual void iterateComponents(
			const std::function<void(T&)>& callback, bool onlyEnabled = false
		) = 0;
	};


	/**
	 * Class ComponentTable, it holds all the Components with type @tparam T
	 * and their relation with the EntityDatabase Entities
	 */
	template <typename T>
	class EntityDatabase::ComponentTable : public ITComponentTable<T>
	{
	private:	// Attributes
		/** The Components added to the ComponentTable */
		T* mComponents;

		/** The size of @see mComponents */
		std::size_t mMaxComponents;

		/** The current number of Components in use in @see mComponents */
		std::size_t mNumComponents;

		/** It holds the flags that tells if a Component in @see mComponents
		 * is in use or enabled */
		std::vector<bool> mComponentFlags;

		/** Maps each Entity with the index of its respecive Component in
		 * @see mComponents */
		std::unordered_map<Entity, std::size_t> mEntityComponentMap;

		/** Maps each Component in @see mComponents with its respecive
		 * Entity */
		std::unordered_map<const T*, Entity> mComponentEntityMap;

	public:		// Functions
		/** Creates a new ComponentTable
		 *
		 * @param	maxComponents the maximum number of Components that the
		 *			ComponentTable can hold */
		ComponentTable(std::size_t maxComponents) :
			mComponents(nullptr),
			mMaxComponents(maxComponents), mNumComponents(0)
		{
			mComponents = std::allocator<T>().allocate(mMaxComponents);
			mComponentFlags.resize(2 * mMaxComponents, false);
			mEntityComponentMap.reserve(mMaxComponents);
			mComponentEntityMap.reserve(mMaxComponents);
		};

		/** @copydoc IComponentTable::getMaxComponents() */
		virtual std::size_t getMaxComponents() const override
		{ return mMaxComponents; };

		/** @copydoc ITComponentTable<T>::addComponent(Entity, T&&) */
		virtual T* addComponent(Entity entity, T&& component) override
		{
			std::size_t componentIndex = mMaxComponents;
			for (std::size_t i = 0; i < mMaxComponents; ++i) {
				if (!mComponentFlags[2 * i]) {
					componentIndex = i;
					break;
				}
			}

			if (componentIndex < mMaxComponents) {
				new (&mComponents[componentIndex]) T(std::move(component));
				++mNumComponents;

				mComponentFlags[2 * componentIndex] = true;
				mComponentFlags[2 * componentIndex + 1] = true;
				mEntityComponentMap.emplace(entity, componentIndex);
				mComponentEntityMap.emplace(&mComponents[componentIndex], entity);

				return &mComponents[componentIndex];
			}

			return nullptr;
		};

		/** @copydoc IComponentTable::copyComponent(Entity, Entity) */
		virtual bool copyComponent(Entity source, Entity destination) override
		{
			auto component = getComponent(source);
			if (component) {
				bool ret = addComponent(destination, T(*component));
				if (!hasComponentEnabled(source)) {
					disableComponent(destination);
				}
				return ret;
			}
			return false;
		};

		/** @copydoc IComponentTable::hasComponent(Entity) */
		virtual bool hasComponent(Entity entity) const override
		{
			return mEntityComponentMap.find(entity) != mEntityComponentMap.end();
		};

		/** @copydoc ITComponentTable<T>::getComponent(Entity) */
		virtual T* getComponent(Entity entity) override
		{
			auto it = mEntityComponentMap.find(entity);
			if (it != mEntityComponentMap.end()) {
				return &mComponents[it->second];
			}
			return nullptr;
		};

		/** @copydoc ITComponentTable<T>::getEntity(const T*) */
		virtual Entity getEntity(const T* component) override
		{
			auto it = mComponentEntityMap.find(component);
			if (it != mComponentEntityMap.end()) {
				return it->second;
			}
			return kNullEntity;
		};

		/** @copydoc IComponentTable::removeComponent(Entity) */
		virtual void removeComponent(Entity entity) override
		{
			auto it1 = mEntityComponentMap.find(entity);
			if (it1 != mEntityComponentMap.end()) {
				auto it2 = mComponentEntityMap.find(&mComponents[it1->second]);

				mComponents[it1->second].~T();
				--mNumComponents;

				mComponentFlags[2 * it1->second] = false;
				mComponentFlags[2 * it1->second + 1] = false;
				mEntityComponentMap.erase(it1);
				mComponentEntityMap.erase(it2);
			}
		};

		/** @copydoc ITComponentTable<T>::iterateComponents(
		 * const std::function<void(T&)>&, bool) */
		virtual void iterateComponents(
			const std::function<void(T&)>& callback, bool onlyEnabled = false
		) override
		{
			for (std::size_t i = 0, j = 0; i < mMaxComponents; ++i) {
				if (mComponentFlags[2 * i]) {
					if (!onlyEnabled || mComponentFlags[2 * i + 1]) {
						callback(mComponents[i]);
					}

					if (++j >= mNumComponents) {
						break;
					}
				}
			}
		};

		/** @copydoc IComponentTable::enableComponent(Entity) */
		virtual void enableComponent(Entity entity) override
		{
			auto it = mEntityComponentMap.find(entity);
			if (it != mEntityComponentMap.end()) {
				mComponentFlags[2 * it->second + 1] = true;
			}
		};

		/** @copydoc IComponentTable::hasComponentEnabled(Entity) */
		virtual bool hasComponentEnabled(Entity entity) const override
		{
			auto it = mEntityComponentMap.find(entity);
			if (it != mEntityComponentMap.end()) {
				return mComponentFlags[2 * it->second + 1];
			}
			return false;
		};

		/** @copydoc IComponentTable::disableComponent(Entity) */
		virtual void disableComponent(Entity entity) override
		{
			auto it = mEntityComponentMap.find(entity);
			if (it != mEntityComponentMap.end()) {
				mComponentFlags[2 * it->second + 1] = false;
			}
		};
	};


	template <typename T>
	void EntityDatabase::addComponentTable(std::size_t maxComponents)
	{
		std::scoped_lock lock(mEntityDBMutex);

		std::size_t id = getComponentTypeId<T>();
		while (id >= mComponentTables.size()) {
			mComponentTables.emplace_back(nullptr);
		}

		mComponentTables[id] = std::make_unique<ComponentTable<T>>(maxComponents);
	}


	template <typename F>
	EntityDatabase& EntityDatabase::executeQuery(F&& callback)
	{
		std::scoped_lock lock(mEntityDBMutex);

		Query query(*this);
		callback(query);
		return *this;
	}


	template <typename T>
	Entity EntityDatabase::Query::getEntity(const T* component)
	{
		return mParent.getTable<T>().getEntity(component);
	}


	template <typename F>
	void EntityDatabase::Query::iterateEntities(F&& callback)
	{
		for (Entity entity = 1; entity <= mParent.mLastEntity; ++entity) {
			if (mParent.mRemovedEntities.find(entity) == mParent.mRemovedEntities.end()) {
				callback(entity);
			}
		}
	}


	template <typename T, typename... Args>
	T* EntityDatabase::Query::emplaceComponent(Entity entity, bool enabled, Args&&... args)
	{
		T component(std::forward<Args>(args)...);
		return addComponent(entity, std::move(component), enabled);
	}


	template <typename T>
	T* EntityDatabase::Query::addComponent(Entity entity, T&& component, bool enabled)
	{
		if (entity == kNullEntity) { return nullptr; }

		auto& table = mParent.getTable<T>();
		T* ret = table.addComponent(entity, std::forward<T>(component));
		if (!enabled) {
			table.disableComponent(entity);
		}

		if (table.hasComponentEnabled(entity)) {
			for (auto& pair : mParent.mSystems) {
				if (pair.second.get<T>()) {
					pair.first->onNewComponent(entity, ComponentMask().set<T>(true), *this);
				}
			}
		}

		return ret;
	}


	template <typename T>
	T* EntityDatabase::Query::copyComponent(Entity source, Entity destination)
	{
		auto& table = mParent.getTable<T>();
		if (table.copyComponent(source, destination)) {
			if (table.hasComponentEnabled(destination)) {
				for (auto& pair : mParent.mSystems) {
					if (pair.second.get<T>()) {
						pair.first->onNewComponent(destination, ComponentMask().set<T>(true), *this);
					}
				}
			}

			return table.getComponent(destination);
		}
		return nullptr;
	}


	template <typename T1, typename T2, typename... Args>
	bool EntityDatabase::Query::hasComponents(Entity entity)
	{
		return hasComponents<T1>(entity)
			&& hasComponents<T2, Args...>(entity);
	}


	template <typename T>
	bool EntityDatabase::Query::hasComponents(Entity entity)
	{
		return mParent.getTable<T>().hasComponent(entity);
	}


	template <typename T1, typename T2, typename... Args>
	std::tuple<T1*, T2*, Args*...> EntityDatabase::Query::getComponents(Entity entity, bool onlyEnabled)
	{
		auto [component] = getComponents<T1>(entity, onlyEnabled);
		return std::tuple_cat(std::make_tuple(component), getComponents<T2, Args...>(entity, onlyEnabled));
	}


	template <typename T>
	std::tuple<T*> EntityDatabase::Query::getComponents(Entity entity, bool onlyEnabled)
	{
		T* component = nullptr;

		auto& table = mParent.getTable<T>();
		if (table.hasComponent(entity) && (!onlyEnabled || table.hasComponentEnabled(entity))) {
			component = table.getComponent(entity);
		}

		return std::make_tuple(component);
	}


	template <typename T>
	void EntityDatabase::Query::iterateComponents(const std::function<void(T&)>& callback, bool onlyEnabled)
	{
		mParent.getTable<T>().iterateComponents(callback, onlyEnabled);
	}


	template <typename... Args, typename F>
	void EntityDatabase::Query::iterateEntityComponents(F&& callback, bool onlyEnabled)
	{
		iterateEntities([&](Entity entity) {
			if (hasComponents<Args...>(entity)
				&& (!onlyEnabled || hasComponentsEnabled<Args...>(entity))
			) {
				auto components = getComponents<Args...>(entity);
				auto params = std::tuple_cat(std::make_tuple(entity), components);
				std::apply(callback, params);
			}
		});
	}


	template <typename T>
	void EntityDatabase::Query::removeComponent(Entity entity)
	{
		auto& table = mParent.getTable<T>();
		if (table.hasComponent(entity)) {
			if (table.hasComponentEnabled(entity)) {
				for (auto& pair : mParent.mSystems) {
					if (pair.second.get<T>()) {
						pair.first->onRemoveComponent(entity, ComponentMask().set<T>(true), *this);
					}
				}
			}

			table.removeComponent(entity);
		}
	}


	template <typename T>
	void EntityDatabase::Query::enableComponents(Entity entity)
	{
		auto& table = mParent.getTable<T>();
		if (table.hasComponent(entity)) {
			table.enableComponent(entity);

			if (table.hasComponentEnabled(entity)) {
				for (auto& pair : mParent.mSystems) {
					if (pair.second.get<T>()) {
						pair.first->onNewComponent(entity, ComponentMask().set<T>(true), *this);
					}
				}
			}
		}
	}


	template <typename T1, typename T2, typename... Args>
	void EntityDatabase::Query::enableComponents(Entity entity)
	{
		enableComponents<T1>(entity);
		enableComponents<T2, Args...>(entity);
	}


	template <typename T1, typename T2, typename... Args>
	bool EntityDatabase::Query::hasComponentsEnabled(Entity entity)
	{
		return hasComponentsEnabled<T1>(entity)
			&& hasComponentsEnabled<T2, Args...>(entity);
	}


	template <typename T>
	bool EntityDatabase::Query::hasComponentsEnabled(Entity entity)
	{
		return mParent.getTable<T>().hasComponentEnabled(entity);
	}


	template <typename T>
	void EntityDatabase::Query::disableComponents(Entity entity)
	{
		auto& table = mParent.getTable<T>();
		if (table.hasComponent(entity)) {
			if (table.hasComponentEnabled(entity)) {
				for (auto& pair : mParent.mSystems) {
					if (pair.second.get<T>()) {
						pair.first->onRemoveComponent(entity, ComponentMask().set<T>(true), *this);
					}
				}
			}

			table.disableComponent(entity);
		}
	}


	template <typename T1, typename T2, typename... Args>
	void EntityDatabase::Query::disableComponents(Entity entity)
	{
		disableComponents<T1>(entity);
		disableComponents<T2, Args...>(entity);
	}

// Private functions
	template <typename T>
	std::size_t EntityDatabase::getComponentTypeId()
	{
		static std::size_t sComponentId = sComponentTypeCount++;
		return sComponentId;
	}


	template <typename T>
	EntityDatabase::ITComponentTable<T>& EntityDatabase::getTable() const
	{
		return *static_cast<ITComponentTable<T>*>( mComponentTables[getComponentTypeId<T>()].get() );
	}

}

#endif		// ECS_HPP
