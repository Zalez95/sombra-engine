#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <se/app/ScriptComponent.h>
#include <se/app/graphics/Material.h>
#include <se/graphics/3D/Mesh.h>
#include <se/graphics/2D/RenderableText.h>

namespace game {

	class Level;


	/**
	 * Class PlayerController, TODO:
	 */
	class PlayerController : public se::app::ScriptComponent
	{
	private:	// Attributes
		Level& mLevel;
		se::graphics::RenderableText& mPickText;

		static constexpr float kRunSpeed	= 2.5f;
		static constexpr float kJumpSpeed	= 3.0f;
		static constexpr float kMouseSpeed	= 100.0f;
		static constexpr float kPitchLimit	= 0.05f;

		std::shared_ptr<se::graphics::Mesh> mTetrahedronMesh;
		std::shared_ptr<se::graphics::Technique> mYellowTechnique;

	public:		// Functions
		/** Creates a new PlayerController
		 *
		 * @param	level the Level that holds the player Entity
		 * @param	pickText the text to write to */
		PlayerController(Level& level, se::graphics::RenderableText& pickText);

		/** @copydoc se::app::ScriptComponent::onCreate() */
		virtual void onCreate(const se::app::UserInput& userInput) override;

		/** @copydoc se::app::ScriptComponent::onDestroy() */
		virtual void onDestroy(const se::app::UserInput& userInput) override;

		/** @copydoc se::app::ScriptComponent::onUpdate(
		 * float, const se::app::UserInput&) */
		virtual void onUpdate(
			float elapsedTime, const se::app::UserInput& userInput
		) override;
	private:
		/** Resets the mouse position to the center of the window
		 *
		 * @param	width the Screen width
		 * @param	height the Screen height */
		void resetMousePosition(double width, double height);
	};

}

#endif		// PLAYER_CONTROLLER_H
