#include "se/app/ECS.h"

namespace se::app {

	std::size_t EntityDatabase::sComponentTypeCount = 0;


	EntityDatabase::EntityDatabase(std::size_t maxEntities) :
		mMaxEntities(maxEntities), mLastEntity(kNullEntity)
	{
		mRemovedEntities.reserve(mMaxEntities);
	}


	EntityDatabase::~EntityDatabase()
	{
		executeQuery([](Query& query) { query.clearEntities(); });
	}


	void EntityDatabase::addSystem(ISystem* system, const ComponentMask& mask)
	{
		std::scoped_lock lock(mEntityDBMutex);

		auto itSystem = std::find_if(mSystems.begin(), mSystems.end(), [&](const auto& pair) {
			return pair.first == system;
		});
		if (itSystem != mSystems.end()) {
			itSystem->second = mask;
		}
		else {
			mSystems.emplace_back(system, mask);
		}
	}


	EntityDatabase::ComponentMask EntityDatabase::getSystemMask(ISystem* system)
	{
		std::scoped_lock lock(mEntityDBMutex);

		ComponentMask ret;
		auto itSystem = std::find_if(mSystems.begin(), mSystems.end(), [&](const auto& pair) {
			return pair.first == system;
		});
		if (itSystem != mSystems.end()) {
			ret = itSystem->second;
		}
		return ret;
	}


	void EntityDatabase::removeSystem(ISystem* system)
	{
		std::scoped_lock lock(mEntityDBMutex);

		mSystems.erase(
			std::remove_if(mSystems.begin(), mSystems.end(), [&](const auto& pair) {
				return pair.first == system;
			}),
			mSystems.end()
		);
	}


	Entity EntityDatabase::Query::addEntity()
	{
		if (mParent.mRemovedEntities.empty()) {
			if (mParent.mLastEntity < static_cast<Entity>(mParent.mMaxEntities)) {
				return ++mParent.mLastEntity;
			}
		}
		else {
			auto it = mParent.mRemovedEntities.begin();
			Entity ret = *it;
			mParent.mRemovedEntities.erase(it);

			return ret;
		}

		return kNullEntity;
	}


	Entity EntityDatabase::Query::copyEntity(Entity source)
	{
		Entity ret = addEntity();

		for (std::size_t i = 0; i < mParent.mComponentTables.size(); ++i) {
			if (mParent.mComponentTables[i]->copyComponent(source, ret)) {
				if (mParent.mComponentTables[i]->hasComponentEnabled(ret)) {
					for (auto& pair : mParent.mSystems) {
						if (pair.second[i]) {
							pair.first->onNewComponent(ret, ComponentMask().set(i, true), *this);
						}
					}
				}
			}
		}

		return ret;
	}


	void EntityDatabase::Query::removeEntity(Entity entity)
	{
		if (entity == kNullEntity) { return; }

		for (std::size_t i = 0; i < mParent.mComponentTables.size(); ++i) {
			if (mParent.mComponentTables[i]->hasComponentEnabled(entity)) {
				for (auto& pair : mParent.mSystems) {
					if (pair.second[i]) {
						pair.first->onRemoveComponent(entity, ComponentMask().set(i, true), *this);
					}
				}
			}

			mParent.mComponentTables[i]->removeComponent(entity);
		}

		mParent.mRemovedEntities.emplace(entity);
	}


	void EntityDatabase::Query::clearEntities()
	{
		iterateEntities([this](Entity entity) {
			removeEntity(entity);
		});
	}

}
