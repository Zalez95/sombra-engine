#ifndef NPC_SCRIPT_H
#define NPC_SCRIPT_H

#include <se/app/ScriptComponent.h>

namespace game {

	/**
	 * Class CharacterScript, it's the Script used for controlling
	 * for the player Entity as a FPS
	 */
	class CharacterScript : public se::app::Script
	{
	protected:	// Nested types
		struct CharacterData
		{
			se::app::Entity floorCollisionEntity;
		};

	public:		// Functions
		/** Class destructor */
		virtual ~CharacterScript() = default;

		/** @copydoc se::app::Script::clone() */
		virtual std::unique_ptr<se::app::Script> clone() const override;

		/** @copydoc se::app::Script::onAdd(se::app::Entity) */
		virtual void onAdd(
			se::app::Entity entity,
			const se::app::ScriptSharedState& sharedState
		) override;

		/** @copydoc se::app::Script::onUpdate(
		 * se::app::Entity, const se::app::ScriptSharedState&) */
		virtual void onUpdate(
			se::app::Entity entity,
			const se::app::ScriptSharedState& sharedState
		) override;

		/** @copydoc se::app::Script::onRemove(se::app::Entity) */
		virtual void onRemove(
			se::app::Entity entity,
			const se::app::ScriptSharedState& sharedState
		) override;
	};

}

#endif		// NPC_SCRIPT_H
