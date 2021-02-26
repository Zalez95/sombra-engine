#ifndef VIEWPORT_CONTROL_H
#define VIEWPORT_CONTROL_H

#include <se/app/ScriptComponent.h>
#include <se/app/TransformsComponent.h>

namespace editor {

	/**
	 * Class ViewportControl, Implements Blender-like viewport controls for a
	 * Entity Camera
	 */
	class ViewportControl : public se::app::ScriptComponent
	{
	private:	// Attributes
		static constexpr float kMoveSpeed		= 25.0f;
		static constexpr float kRotationSpeed	= 10.0f;
		static constexpr float kPitchLimit		= 0.05f;

		/** The last mouse location */
		float mLastMouseX = 0.0f, mLastMouseY = 0.0f;

		/** The current zoom value */
		float mZoom = 10.0f;

	public:		// Functions
		/** @copydoc se::app::ScriptComponent::onUpdate(
		 * float, const se::app::UserInput&) */
		virtual void onUpdate(
			float elapsedTime, const se::app::UserInput& userInput
		) override;
	private:
		void zoom(
			const se::app::UserInput& userInput,
			se::app::TransformsComponent& transforms
		);
		void move(
			const se::app::UserInput& userInput,
			se::app::TransformsComponent& transforms
		);
		void orbit(
			const se::app::UserInput& userInput,
			se::app::TransformsComponent& transforms
		);
	};

}

#endif		// VIEWPORT_CONTROL_H
