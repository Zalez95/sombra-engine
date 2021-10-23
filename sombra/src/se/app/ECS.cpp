#include "se/app/ECS.h"

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


	EntityDatabase::ComponentMask EntityDatabase::getSystemMask(ISystem* system) const
	{
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


	Entity EntityDatabase::copyEntity(Entity source)
	{
		Entity ret = addEntity();

		std::size_t numComponents = mComponentTables.size();
		for (std::size_t i = 0; i < numComponents; ++i) {
			std::size_t srcComponentsBaseIndex = 2 * (source * mComponentTables.size() + i);
			std::size_t desComponentsBaseIndex = 2 * (ret * mComponentTables.size() + i);

			mActiveComponents[desComponentsBaseIndex + 1] = mActiveComponents[srcComponentsBaseIndex + 1];
			if (mActiveComponents[srcComponentsBaseIndex]) {
				if (mComponentTables[i]->copyComponent(source, ret)) {
					mActiveComponents[desComponentsBaseIndex] = true;

					if (mActiveComponents[desComponentsBaseIndex + 1]) {
						for (auto& pair : mSystems) {
							if (pair.second[i]) {
								pair.first->onNewComponent(ret, ComponentMask().set(i, true));
							}
						}
					}
				}
			}
		}

		return ret;
	}


	void EntityDatabase::removeEntity(Entity entity)
	{
		std::size_t numComponents = mComponentTables.size();
		for (std::size_t i = 0; i < numComponents; ++i) {
			std::size_t activeComponentsBaseIndex = 2 * (entity * mComponentTables.size() + i);
			if (mActiveComponents[activeComponentsBaseIndex]) {
				if (mActiveComponents[activeComponentsBaseIndex + 1]) {
					for (auto& pair : mSystems) {
						if (pair.second[i]) {
							pair.first->onRemoveComponent(entity, ComponentMask().set(i, true));
						}
					}
				}

				mComponentTables[i]->removeComponent(entity);
			}

			mActiveComponents[activeComponentsBaseIndex] = false;
			mActiveComponents[activeComponentsBaseIndex + 1] = true;
		}

		if (entity != kNullEntity) {
			mRemovedEntities.emplace(entity);
		}
	}


	void EntityDatabase::clearEntities()
	{
		iterateEntities([this](Entity entity) {
			removeEntity(entity);
		});
	}

}
