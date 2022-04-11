#include <se/app/TransformsComponent.h>
#include "NPCScript.h"

namespace game {

	std::unique_ptr<se::app::Script> NPCScript::clone() const
	{
		return std::make_unique<NPCScript>(*this);
	}


	void NPCScript::onUpdate(se::app::Entity entity, const se::app::ScriptSharedState& sharedState)
	{
		sharedState.entityDatabase->executeQuery([&](se::app::EntityDatabase::Query& query) {
			auto [transforms] = query.getComponents<se::app::TransformsComponent>(entity, true);
			if (!transforms) { return; }

			mTimeSinceStart += sharedState.deltaTime;
			if (mTimeSinceStart >= 5.0f) {
				mTimeSinceStart = 0.0f;

				glm::quat qPitch = glm::angleAxis(glm::half_pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
				transforms->orientation = glm::normalize(transforms->orientation * qPitch);
			}

			glm::vec3 forward = transforms->orientation * glm::vec3(0.0f, 0.0f,-1.0f);
			transforms->velocity += kRunSpeed * sharedState.deltaTime * forward;
			transforms->updated.reset();
		});
	}

}
