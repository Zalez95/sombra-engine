#ifndef EDITOR_H
#define EDITOR_H

#include <se/app/Application.h>
#include <se/app/Scene.h>
#include "ImGuiInput.h"
#include "MenuBar.h"
#include "EntityPanel.h"

namespace editor {

	/**
	 * Class Editor, TODO:
	 */
	class Editor : public se::app::Application
	{
	private:	// Attributes
		static constexpr char kTitle[]							= "SOMBRA EDITOR";
		static constexpr unsigned int kWidth					= 1280;
		static constexpr unsigned int kHeight					= 720;
		static constexpr std::size_t kMaxManifolds				= 128;
		static constexpr float kMinFDifference					= 0.00001f;
		static constexpr std::size_t kMaxCollisionIterations	= 128;
		static constexpr float kContactPrecision				= 0.0000001f;
		static constexpr float kContactSeparation				= 0.00001f;
		static constexpr int kMaxRayCasterIterations			= 32;
		static constexpr float kUpdateTime						= 0.016f;
		se::app::Scene* mScene;
		ImGuiInput* mImGuiInput;
		MenuBar* mMenuBar;
		EntityPanel* mEntityPanel;

	public:		// Functions
		/** Creates a new Editor */
		Editor();

		/** Class destructor */
		~Editor();

		/** @return	a pointer to the Scene of the Editor */
		se::app::Scene* getScene() { return mScene; };

		/** Creates a new Scene */
		void createScene(const char* name = "");

		/** Destroy the current Scene */
		void destroyScene();
	protected:
		/** Updates the Editor managers and systems each main loop
		 * iteration
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		virtual void onUpdate(float deltaTime) override;

		/** Draws to screen */
		virtual void onRender() override;
	};

}

#endif		// EDITOR_H
