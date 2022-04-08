#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <se/graphics/3D/Mesh.h>
#include <se/graphics/2D/RenderableText.h>
#include <se/app/graphics/RenderableShader.h>
#include <se/app/LightComponent.h>
#include <se/app/ScriptComponent.h>

namespace game {

	class Level;


	/**
	 * Class PlayerController, it's the Script used for controlling
	 * for the player Entity as a FPS
	 */
	class PlayerController : public se::app::Script
	{
	private:	// Attributes
		static constexpr float kRunSpeed	= 100.0f;
		static constexpr float kJumpSpeed	= 150.0f;
		static constexpr float kMouseSpeed	= 100.0f;
		static constexpr float kPitchLimit	= 0.05f;

		Level& mLevel;
		se::graphics::RenderableText& mPickText;

		/** The last mouse location */
		float mLastMouseX = 0.0f, mLastMouseY = 0.0f;

		se::app::Repository::ResourceRef<se::app::MeshRef>
			mTetrahedronMesh;
		se::app::Repository::ResourceRef<se::app::RenderableShader>
			mShaderYellow;
		se::app::Repository::ResourceRef<se::app::LightSource>
			mLightYellow;

	public:		// Functions
		/** Creates a new PlayerController
		 *
		 * @param	level the Level that holds the player Entity
		 * @param	pickText the text to write to */
		PlayerController(Level& level, se::graphics::RenderableText& pickText);

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
	};

}

#endif		// PLAYER_CONTROLLER_H
