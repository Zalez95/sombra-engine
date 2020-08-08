#include "se/app/EntityDatabase.h"

namespace se::app {

	std::size_t EntityDatabase::sComponentTypeCount = 0;


	EntityDatabase::ComponentMask::ComponentMask(bool value) :
		mBitMask(sComponentTypeCount, value) {}


	EntityDatabase::EntityDatabase(std::size_t maxEntities) :
		mMaxEntities(maxEntities), mLastEntity(kNullEntity)
	{
		mRemovedEntities.reserve(mMaxEntities);
	}


	EntityDatabase::~EntityDatabase()
	{
		iterateEntities([this](Entity entity) {
			removeEntity(entity);
		});
	}


	void EntityDatabase::addSystem(ISystem* system, ComponentMask mask)
	{
		mSystems.emplace_back(system, mask);
	}


	void EntityDatabase::removeSystem(ISystem* system)
	{
		mSystems.erase(
			std::remove_if(mSystems.begin(), mSystems.end(), [&](const auto& pair) {
				return pair.first == system;
			}),
			mSystems.end()
		);
	}


	Entity EntityDatabase::addEntity()
	{
		if (mRemovedEntities.empty()) {
			if (mLastEntity < static_cast<Entity>(mMaxEntities)) {
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


	void EntityDatabase::removeEntity(Entity entity)
	{
		std::size_t numComponents = mComponentTables.size();
		for (std::size_t i = 0; i < numComponents; ++i) {
			if (mActiveComponents[entity * numComponents + i]) {
				for (auto& pair : mSystems) {
					if (pair.second[i]) {
						pair.first->onRemoveEntity(entity);
					}
				}

				mActiveComponents[entity * numComponents + i] = false;
				mComponentTables[i]->removeComponent(entity);
			}
		}

		mRemovedEntities.emplace(entity);
	}

}
