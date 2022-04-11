#include <se/app/TransformsComponent.h>
#include <se/app/RigidBodyComponent.h>
#include <se/app/AnimationComponent.h>
#include "CharacterScript.h"

namespace game {

	std::unique_ptr<se::app::Script> CharacterScript::clone() const
	{
		return std::make_unique<CharacterScript>(*this);
	}


	void CharacterScript::onAdd(se::app::Entity entity, const se::app::ScriptSharedState& sharedState)
	{
		sharedState.entityDatabase->executeQuery([&](se::app::EntityDatabase::Query& query) {
			auto [scriptC] = query.getComponents<se::app::ScriptComponent>(entity, true);
			if (scriptC) {
				se::app::Entity collisionEntity = query.addEntity();
				query.emplaceComponent<se::app::TransformsComponent>(collisionEntity);
				query.emplaceComponent<se::app::RigidBodyComponent>(collisionEntity);
				auto animationC2 = query.emplaceComponent<se::app::AnimationComponent>(collisionEntity);

				auto [animationC1] = query.getComponents<se::app::AnimationComponent>(entity, true);
				if (!animationC1) {
					animationC1 = query.emplaceComponent<se::app::AnimationComponent>(collisionEntity);
				}

				scriptC->setScriptData(new CharacterData{ collisionEntity });
			}
		});
	}


	void CharacterScript::onUpdate(se::app::Entity entity, const se::app::ScriptSharedState& sharedState)
	{
		sharedState.entityDatabase->executeQuery([&](se::app::EntityDatabase::Query& query) {

		});
	}


	void CharacterScript::onRemove(se::app::Entity entity, const se::app::ScriptSharedState& sharedState)
	{
		sharedState.entityDatabase->executeQuery([&](se::app::EntityDatabase::Query& query) {
			auto [scriptC] = query.getComponents<se::app::ScriptComponent>(entity, true);
			if (scriptC) {
				CharacterData* cData = static_cast<CharacterData*>(scriptC->getScriptData());
				query.removeEntity(cData->floorCollisionEntity);
				delete cData;
				scriptC->setScriptData(nullptr);
			}
		});
	}

}
