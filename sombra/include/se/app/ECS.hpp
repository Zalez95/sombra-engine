#ifndef ECS_HPP
#define ECS_HPP

#include <cassert>
#include <functional>
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
		 *			nullptr otherwise */
		virtual T* addComponent(Entity entity, T&& component) = 0;

		/** Adds a Component to the ITComponentTable and makes the given Entity
		 * its owner
		 *
		 * @param	entity the Entity that will own the Component
		 * @param	component a pointer to the Component to add
		 * @return	a pointer to tha Component if it was added successfully,
		 *			nullptr otherwise */
		virtual T* addComponent(Entity entity, std::unique_ptr<T> component) = 0;

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
		 * @param	callback the function to call for each Component */
		virtual void iterateComponents(std::function<void(T&)> callback) = 0;
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
		utils::PackedVector<T> mComponents;

		/** Maps each Entity with the index of its respecive Component in
		 * @see mComponents */
		std::unordered_map<Entity, std::size_t> mEntityComponentMap;

		/** Maps each Component index in @see mComponents with its respecive
		 * Entity */
		std::unordered_map<const T*, Entity> mComponentEntityMap;

		/** The function used for copying the Components */
		CopyCallback<T> mCopyCallback;

	public:		// Functions
		/** Creates a new ComponentTable
		 *
		 * @param	maxComponents the maximum number of Components that the
		 *			ComponentTable can hold
		 * @param	copyCB the function used for copying the Components */
		ComponentTable(std::size_t maxComponents, const CopyCallback<T>& copyCB) :
			mCopyCallback(copyCB)
		{
			mComponents.reserve(maxComponents);
			mEntityComponentMap.reserve(maxComponents);
			mComponentEntityMap.reserve(maxComponents);
		};

		/** @copydoc IComponentTable::getMaxComponents() */
		virtual std::size_t getMaxComponents() const override
		{ return mComponents.capacity(); };

		/** @copydoc ITComponentTable<T>::addComponent(Entity, T&&) */
		virtual T* addComponent(Entity entity, T&& component) override
		{
			if (mComponents.size() < mComponents.capacity()) {
				auto it = mComponents.emplace(std::forward<T>(component));
				mEntityComponentMap.emplace(entity, it.getIndex());
				mComponentEntityMap.emplace(&(*it), entity);
				return &(*it);
			}
			return nullptr;
		};

		/** @copydoc ITComponentTable<T>::addComponent(Entity, std::unique_ptr<T>) */
		virtual T* addComponent(Entity entity, std::unique_ptr<T> component) override
		{
			if (mComponents.size() < mComponents.capacity()) {
				auto it = mComponents.emplace(std::move(*component));
				mEntityComponentMap.emplace(entity, it.getIndex());
				mComponentEntityMap.emplace(&(*it), entity);
				return &(*it);
			}
			return nullptr;
		};

		/** @copydoc IComponentTable::copyComponent(Entity, Entity) */
		virtual bool copyComponent(Entity source, Entity destination) override
		{
			if (mCopyCallback) {
				auto component = getComponent(source);
				if (component) {
					return addComponent(destination, mCopyCallback(*component));
				}
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
				mComponents.erase( mComponents.begin().setIndex(it1->second) );
				mEntityComponentMap.erase(it1);
				mComponentEntityMap.erase(it2);
			}
		};

		/** @copydoc ITComponentTable<T>::iterateComponents(std::function<void(T&)>) */
		virtual void iterateComponents(std::function<void(T&)> callback) override
		{
			for (auto& component : mComponents) {
				callback(component);
			}
		};
	};


	/**
	 * Class ComponentUPtrTable, it holds all the Components with type @tparam T
	 * and their Derived classes and their relation with the EntityDatabase
	 * Entities
	 * @note	The Components wont be stored contiguously in memory
	 */
	template <typename T>
	class EntityDatabase::ComponentTableUPtr : public ITComponentTable<T>
	{
	private:	// Attributes
		/** The Component pointers added to the ComponentTableUPtr */
		utils::PackedVector<std::unique_ptr<T>> mComponents;

		/** Maps each Entity with the index of its respecive Component in
		 * @see mComponents */
		std::unordered_map<Entity, std::size_t> mEntityComponentMap;

		/** Maps each Component index in @see mComponents with its respecive
		 * Entity */
		std::unordered_map<const T*, Entity> mComponentEntityMap;

		/** The function used for copying the Components */
		CopyCallbackUPtr<T> mCopyCallback;

	public:		// Functions
		/** Creates a new ComponentTableUPtr
		 *
		 * @param	maxComponents the maximum number of Components that the
		 *			ComponentTableUPtr can hold
		 * @param	copyCB the function used for copying the Components */
		ComponentTableUPtr(std::size_t maxComponents, const CopyCallbackUPtr<T>& copyCB) :
			mCopyCallback(copyCB)
		{
			mComponents.reserve(maxComponents);
			mEntityComponentMap.reserve(maxComponents);
			mComponentEntityMap.reserve(maxComponents);
		};

		/** @copydoc IComponentTable::getMaxComponents() */
		virtual std::size_t getMaxComponents() const override
		{ return mComponents.capacity(); };

		/** @copydoc ITComponentTable<T>::addComponent(Entity, T&&) */
		virtual T* addComponent(Entity, T&&) override
		{
			assert(false && "This function can't be used with virtual classes");
			return nullptr;
		};

		/** @copydoc ITComponentTable<T>::addComponent(Entity, std::unique_ptr<T>) */
		virtual T* addComponent(Entity entity, std::unique_ptr<T> component) override
		{
			if (mComponents.size() < mComponents.capacity()) {
				auto it = mComponents.emplace(std::move(component));
				mEntityComponentMap.emplace(entity, it.getIndex());
				mComponentEntityMap.emplace(it->get(), entity);
				return it->get();
			}
			return nullptr;
		};

		/** @copydoc IComponentTable::copyComponent(Entity, Entity) */
		virtual bool copyComponent(Entity source, Entity destination) override
		{
			if (mCopyCallback) {
				auto component = getComponent(source);
				if (component) {
					return addComponent(destination, mCopyCallback(*component));
				}
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
				return mComponents[it->second].get();
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
				auto it2 = mComponentEntityMap.find(mComponents[it1->second].get());
				mComponents.erase( mComponents.begin().setIndex(it1->second) );
				mEntityComponentMap.erase(it1);
				mComponentEntityMap.erase(it2);
			}
		};

		/** @copydoc ITComponentTable<T>::iterateComponents(std::function<void(T&)>) */
		virtual void iterateComponents(std::function<void(T&)> callback) override
		{
			for (auto& component : mComponents) {
				callback(*component);
			}
		};
	};


	template <typename T>
	EntityDatabase::ComponentMask& EntityDatabase::ComponentMask::set(bool value)
	{
		mBitMask[getComponentTypeId<T>()] = value;
		return *this;
	}


	template <typename T>
	bool EntityDatabase::ComponentMask::get() const
	{
		return mBitMask[getComponentTypeId<T>()];
	}


	template <typename T, bool hasDerived>
	std::enable_if_t<!hasDerived, void> EntityDatabase::addComponentTable(
		std::size_t maxComponents, const CopyCallback<T>& copyCB
	) {
		std::size_t id = getComponentTypeId<T>();
		while (id >= mComponentTables.size()) {
			mComponentTables.emplace_back(nullptr);
		}

		mComponentTables[id] = std::make_unique<ComponentTable<T>>(maxComponents, copyCB);

		std::size_t activeComponentsSize = 2 * mMaxEntities * mComponentTables.size();
		mActiveComponents.reserve(activeComponentsSize);
		for (std::size_t i = 0; i < activeComponentsSize; ++i) {
			mActiveComponents.push_back(i % 2 == 1);
		}
	}


	template <typename T, bool hasDerived>
	std::enable_if_t<hasDerived, void> EntityDatabase::addComponentTable(
		std::size_t maxComponents, const CopyCallbackUPtr<T>& copyCB
	) {
		std::size_t id = getComponentTypeId<T>();
		while (id >= mComponentTables.size()) {
			mComponentTables.emplace_back(nullptr);
		}

		mComponentTables[id] = std::make_unique<ComponentTableUPtr<T>>(maxComponents, copyCB);

		std::size_t activeComponentsSize = 2 * mMaxEntities * mComponentTables.size();
		mActiveComponents.reserve(activeComponentsSize);
		for (std::size_t i = 0; i < activeComponentsSize; ++i) {
			mActiveComponents.push_back(i % 2 == 1);
		}
	}


	template <typename T>
	Entity EntityDatabase::getEntity(const T* component)
	{
		return getTable<T>().getEntity(component);
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


	template <typename T>
	std::size_t EntityDatabase::getMaxComponents() const
	{
		return getTable<T>().getMaxComponents();
	}


	template <typename T, typename... Args>
	T* EntityDatabase::emplaceComponent(Entity entity, Args&&... args)
	{
		T component(std::forward<Args>(args)...);
		return addComponent(entity, std::move(component));
	}


	template <typename T>
	T* EntityDatabase::addComponent(Entity entity, T&& component)
	{
		T* ret = nullptr;
		if (entity != kNullEntity) {
			ret = getTable<T>().addComponent(entity, std::forward<T>(component));

			std::size_t activeComponentsBaseIndex = 2 * (entity * mComponentTables.size() + getComponentTypeId<T>());
			mActiveComponents[activeComponentsBaseIndex] = true;
			if (mActiveComponents[activeComponentsBaseIndex + 1]) {
				for (auto& pair : mSystems) {
					if (pair.second.get<T>()) {
						pair.first->onNewComponent(entity, ComponentMask().set<T>(true));
					}
				}
			}
		}
		return ret;
	}


	template <typename T>
	T* EntityDatabase::addComponent(Entity entity, std::unique_ptr<T> component)
	{
		T* ret = nullptr;
		if (entity != kNullEntity) {
			ret = getTable<T>().addComponent(entity, std::move(component));

			std::size_t activeComponentsBaseIndex = 2 * (entity * mComponentTables.size() + getComponentTypeId<T>());
			mActiveComponents[activeComponentsBaseIndex] = true;
			if (mActiveComponents[activeComponentsBaseIndex + 1]) {
				for (auto& pair : mSystems) {
					if (pair.second.get<T>()) {
						pair.first->onNewComponent(entity, ComponentMask().set<T>(true));
					}
				}
			}
		}
		return ret;
	}


	template <typename T>
	T* EntityDatabase::copyComponent(Entity source, Entity destination)
	{
		std::size_t typeId = getComponentTypeId<T>();
		std::size_t srcComponentsBaseIndex = 2 * (source * mComponentTables.size() + typeId);
		std::size_t desComponentsBaseIndex = 2 * (destination * mComponentTables.size() + typeId);

		if (mActiveComponents[srcComponentsBaseIndex]) {
			const auto& table = getTable<T>();
			if (table.copyComponent(source, destination)) {
				mActiveComponents[desComponentsBaseIndex] = true;
				mActiveComponents[desComponentsBaseIndex + 1] = mActiveComponents[srcComponentsBaseIndex + 1];

				if (mActiveComponents[desComponentsBaseIndex + 1]) {
					for (auto& pair : mSystems) {
						if (pair.second[typeId]) {
							pair.first->onNewComponent(destination, ComponentMask().set(typeId, true));
						}
					}
				}

				return table.getComponent(destination);
			}
		}
		return nullptr;
	}


	template <typename T1, typename T2, typename... Args>
	bool EntityDatabase::hasComponents(Entity entity)
	{
		return mActiveComponents[2 * (entity * mComponentTables.size() + getComponentTypeId<T1>())]
			&& hasComponents<T2, Args...>(entity);
	}


	template <typename T>
	bool EntityDatabase::hasComponents(Entity entity)
	{
		return mActiveComponents[2 * (entity * mComponentTables.size() + getComponentTypeId<T>())];
	}


	template <typename T1, typename T2, typename... Args>
	std::tuple<T1*, T2*, Args*...> EntityDatabase::getComponents(Entity entity, bool onlyEnabled)
	{
		auto [component] = getComponents<T1>(entity, onlyEnabled);
		return std::tuple_cat(std::make_tuple(component), getComponents<T2, Args...>(entity, onlyEnabled));
	}


	template <typename T>
	std::tuple<T*> EntityDatabase::getComponents(Entity entity, bool onlyEnabled)
	{
		T* component = nullptr;
		if (hasComponents<T>(entity) && (!onlyEnabled || hasComponentsEnabled<T>(entity))) {
			component = getTable<T>().getComponent(entity);
		}

		return std::make_tuple(component);
	}


	template <typename... Args, typename F>
	void EntityDatabase::iterateComponents(F&& callback, bool onlyEnabled)
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
	void EntityDatabase::removeComponent(Entity entity)
	{
		std::size_t activeComponentsBaseIndex = 2 * (entity * mComponentTables.size() + getComponentTypeId<T>());
		if (mActiveComponents[activeComponentsBaseIndex]) {
			if (mActiveComponents[activeComponentsBaseIndex + 1]) {
				for (auto& pair : mSystems) {
					if (pair.second.get<T>()) {
						pair.first->onRemoveComponent(entity, ComponentMask().set<T>(true));
					}
				}
			}

			mActiveComponents[activeComponentsBaseIndex] = false;
			getTable<T>().removeComponent(entity);
		}
	}


	template <typename T>
	void EntityDatabase::enableComponent(Entity entity)
	{
		std::size_t activeComponentsBaseIndex = 2 * (entity * mComponentTables.size() + getComponentTypeId<T>());
		if (!mActiveComponents[activeComponentsBaseIndex + 1]) {
			mActiveComponents[activeComponentsBaseIndex + 1] = true;

			if (mActiveComponents[activeComponentsBaseIndex]) {
				for (auto& pair : mSystems) {
					if (pair.second.get<T>()) {
						pair.first->onNewComponent(entity, ComponentMask().set<T>(true));
					}
				}
			}
		}
	}


	template <typename T1, typename T2, typename... Args>
	bool EntityDatabase::hasComponentsEnabled(Entity entity)
	{
		return mActiveComponents[2 * (entity * mComponentTables.size() + getComponentTypeId<T1>()) + 1]
			&& hasComponentsEnabled<T2, Args...>(entity);
	}


	template <typename T>
	bool EntityDatabase::hasComponentsEnabled(Entity entity)
	{
		return mActiveComponents[2 * (entity * mComponentTables.size() + getComponentTypeId<T>()) + 1];
	}


	template <typename T>
	void EntityDatabase::disableComponent(Entity entity)
	{
		std::size_t activeComponentsBaseIndex = 2 * (entity * mComponentTables.size() + getComponentTypeId<T>());
		if (mActiveComponents[activeComponentsBaseIndex + 1]) {
			if (mActiveComponents[activeComponentsBaseIndex]) {
				for (auto& pair : mSystems) {
					if (pair.second.get<T>()) {
						pair.first->onRemoveComponent(entity, ComponentMask().set<T>(true));
					}
				}
			}

			mActiveComponents[activeComponentsBaseIndex + 1] = false;
		}
	}


	template <typename S, typename C>
	void ISystem::tryCallC(
		void(S::*componentHandler)(Entity, C*),
		Entity entity, const EntityDatabase::ComponentMask& mask
	) {
		if (mask.get<C>()) {
			S* thisS = static_cast<S*>(this);

			auto [component] = mEntityDatabase.getComponents<C>(entity);
			(thisS->*componentHandler)(entity, component);
		}
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
