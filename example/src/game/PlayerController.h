#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <se/graphics/3D/Mesh.h>
#include <se/graphics/2D/RenderableText.h>
#include <se/app/graphics/RenderableShader.h>
#include <se/app/ScriptComponent.h>

namespace game {

	class Level;


	/**
	 * Class PlayerController, TODO:
	 */
	class PlayerController : public se::app::ScriptComponent
	{
	private:	// Attributes
		static constexpr float kRunSpeed	= 2.5f;
		static constexpr float kJumpSpeed	= 3.0f;
		static constexpr float kMouseSpeed	= 100.0f;
		static constexpr float kPitchLimit	= 0.05f;

		Level& mLevel;
		se::graphics::RenderableText& mPickText;

		/** The last mouse location */
		float mLastMouseX = 0.0f, mLastMouseY = 0.0f;

		se::app::Repository::ResourceRef<se::graphics::Mesh>
			mTetrahedronMesh;
		se::app::Repository::ResourceRef<se::app::RenderableShader>
			mShaderYellow;

	public:		// Functions
		/** Creates a new PlayerController
		 *
		 * @param	level the Level that holds the player Entity
		 * @param	pickText the text to write to */
		PlayerController(Level& level, se::graphics::RenderableText& pickText);

		/** @copydoc se::app::ScriptComponent::onCreate() */
		virtual void onCreate(const se::app::UserInput& userInput) override;

		/** @copydoc se::app::ScriptComponent::onUpdate(
		 * float, const se::app::UserInput&) */
		virtual void onUpdate(
			float elapsedTime, const se::app::UserInput& userInput
		) override;
	};

}

#endif		// PLAYER_CONTROLLER_H
