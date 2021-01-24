#ifndef EDITOR_H
#define EDITOR_H

#include <se/app/Application.h>
#include <se/app/Scene.h>
#include "ImGuiInput.h"
#include "ImGuiRenderer.h"
#include "MenuBar.h"
#include "EntityPanel.h"
#include "ComponentPanel.h"
#include "RepositoryPanel.h"
#include "ViewportControl.h"

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
		static constexpr float kFOV								= 60.0f;
		static constexpr float kZNear							= 0.1f;
		static constexpr float kZFar							= 2000.0f;

		ImGuiContext* mImGuiContext;
		ImGuiInput* mImGuiInput;
		ImGuiRenderer* mImGuiRenderer;
		MenuBar* mMenuBar;
		EntityPanel* mEntityPanel;
		ComponentPanel* mComponentPanel;
		RepositoryPanel* mRepositoryPanel;

		se::app::Entity mViewportEntity;
		ViewportControl* mViewportControl;

		se::app::Scene* mScene;

	public:		// Functions
		/** Creates a new Editor */
		Editor();

		/** Class destructor */
		virtual ~Editor();

		/** @return	a pointer to the Scene of the Editor */
		se::app::Scene* getScene() const
		{ return mScene; };

		/** @return	the selected entity to work with */
		se::app::Entity getActiveEntity() const
		{ return mEntityPanel->getActiveEntity(); };

		/** Creates a new Scene */
		void createScene(const char* name = "");

		/** Destroy the current Scene */
		void destroyScene();

		/** @copydoc se::app::Application::notify(const IEvent&) */
		virtual void notify(const se::app::IEvent&) override;
	protected:
		/** Updates the Editor Viewport Entity */
		virtual void onInput() override;

		/** Updates the Editor managers and systems each main loop
		 * iteration
		 *
		 * @param	deltaTime the elapsed time since the last update in
		 *			seconds */
		virtual void onUpdate(float deltaTime) override;

		/** Draws to screen */
		virtual void onRender() override;

		/** Handles the given close Event by closing the Editor
		 *
		 * @param	event the ResizeEvent to handle */
		void onCloseEvent(const se::app::Event<se::app::Topic::Close>& event);
	};

}

#endif		// EDITOR_H
