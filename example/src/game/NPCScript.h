#ifndef NPC_SCRIPT_H
#define NPC_SCRIPT_H

#include <se/app/ScriptComponent.h>

namespace game {

	/**
	 * Class NPCScript, it's the Script used for controlling
	 * for the player Entity as a FPS
	 */
	class NPCScript : public se::app::Script
	{
	private:	// Attributes
		static constexpr float kRunSpeed	= 5.0f;
		static constexpr float kLoopTime	= 20.0f;

		float mTimeSinceStart = 0.0f;

	public:		// Functions
		/** @copydoc se::app::Script::clone() */
		virtual std::unique_ptr<se::app::Script> clone() const override;

		/** @copydoc se::app::Script::onUpdate(
		 * se::app::Entity, const se::app::ScriptSharedState&) */
		virtual void onUpdate(
			se::app::Entity entity,
			const se::app::ScriptSharedState& sharedState
		) override;
	};

}

#endif		// NPC_SCRIPT_H
