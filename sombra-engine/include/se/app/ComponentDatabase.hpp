#ifndef COMPONENT_DATABASE_HPP
#define COMPONENT_DATABASE_HPP

namespace se::app {

	template <typename SizeType, typename... Types>
	ComponentDatabase<SizeType, Types...>::ComponentDatabase(SizeType maxEntities) : mMaxEntities(maxEntities)
	{
		mActiveEntities.reserve(mMaxEntities);
		mActiveComponents.reserve(mMaxEntities);
		std::apply([this](auto&& args) { args.reserve(mMaxEntities); }, mComponents);
	}


	template <typename SizeType, typename... Types>
	ComponentDatabase<SizeType, Types...>::EntityId ComponentDatabase<SizeType, Types...>::addEntity()
	{
		EntityId ret = mMaxEntities;

		auto itInactiveEntity = std::find(mActiveEntities.begin(), mActiveEntities.end(), false);
		if (itInactiveEntity != mActiveEntities.end()) {
			*itInactiveEntity = true;
			ret = static_cast<EntityId>( std::distance(itInactiveEntity, mActiveEntities.begin()) );
		}
		else if (mActiveComponents.size() < mMaxEntities) {
			std::apply([this](auto&& args) { args.emplace_back(); }, mComponents);
			mActiveComponents.emplace_back();

			ret = static_cast<EntityId>( mActiveComponents.size() - 1 );
		}

		return ret;
	}


	template <typename SizeType, typename... Types>
	void ComponentDatabase<SizeType, Types...>::removeEntity(EntityId entityId)
	{
		if ((entityId < mActiveComponents.size()) && mActiveEntities[entityId]) {
			mActiveEntities[entityId] = false;
			mActiveComponents[entityId].reset();
			std::apply([&](auto&& args) { args[entityId] = {}; }, mComponents);
		}
	}


	template <typename SizeType, typename... Types>
	template <typename T, unsigned long componentId>
	T* ComponentDatabase<SizeType, Types...>::getComponent(EntityId entityId)
	{
		const T* cComponent = getComponent<T, componentId>(entityId);
		return const_cast<T*>(cComponent);
	}


	template <typename SizeType, typename... Types>
	template <typename T, unsigned long componentId>
	const T* ComponentDatabase<SizeType, Types...>::getComponent(EntityId entityId) const
	{
		T* ret = nullptr;

		if ((entityId < mActiveComponents.size()) && mActiveEntities[entityId]
			&& mActiveComponents[entityId][componentId]
		) {
			ret = &(std::get<componentId>(mComponents)[entityId]);
		}

		return ret;
	}


	template <typename SizeType, typename... Types>
	template <typename T, unsigned long componentId, typename... Args>
	void ComponentDatabase<SizeType, Types...>::addComponent(EntityId entityId, Args&&... args)
	{
		if ((entityId < mActiveComponents.size()) && mActiveEntities[entityId]) {
			mActiveComponents[entityId].set(componentId);
			std::get<componentId>(mComponents)[entityId] = T(std::forward<Args>(args)...);
		}
	}


	template <typename SizeType, typename... Types>
	template <unsigned long componentId>
	void ComponentDatabase<SizeType, Types...>::removeComponent(EntityId entityId)
	{
		if ((entityId < mActiveComponents.size()) && mActiveEntities[entityId]) {
			mActiveComponents[entityId].reset(componentId);
		}
	}


	template <typename SizeType, typename... Types>
	void ComponentDatabase<SizeType, Types...>::processEntities(const EntityCallback& callback, ActiveColumns filters) const
	{
		EntityId entityId = 0;
		while (entityId < mActiveComponents.size()) {
			if (mActiveEntities[entityId] && (filters & mActiveComponents[entityId] == filters) {
				callback(entityId);
			}
		}
	}

}

#endif		// COMPONENT_DATABASE_HPP
