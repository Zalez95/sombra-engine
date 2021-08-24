#ifndef EDITOR_H
#define EDITOR_H

#include <vector>
#include <se/app/Application.h>
#include <se/app/Scene.h>
#include "ImGuiInput.h"
#include "ImGuiRenderer.h"
#include "MenuBar.h"
#include "IEditorPanel.h"

namespace editor {

	/**
	 * Class Editor, it's an Application used for creating, editing and
	 * removing entities and their Components from Scenes. It also allows
	 * modifiyng Scene Resources
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
		static constexpr float kZFar							= 1000.0f;

		ImGuiContext* mImGuiContext;
		ImGuiInput* mImGuiInput;
		ImGuiRenderer* mImGuiRenderer;
		MenuBar* mMenuBar;
		int mPanelIdCount;
		std::vector<IEditorPanel*> mPanels;

		se::app::Entity mViewportEntity;
		se::app::Entity mGridEntity;
		se::app::Entity mActiveEntity;

		se::app::Scene* mScene;

	public:		// Functions
		/** Creates a new Editor */
		Editor();

		/** Class destructor */
		virtual ~Editor();

		/** @return	a pointer to the Scene of the Editor */
		se::app::Scene* getScene() const { return mScene; };

		/** @return	the selected entity to work with */
		se::app::Entity getActiveEntity() const { return mActiveEntity; };

		/** Sets the selected entity to work with
		 *
		 * @param	entity the new active entity */
		void setActiveEntity(se::app::Entity entity)
		{ mActiveEntity = entity; };

		/** @return	the Entity that controls the Editor camera */
		se::app::Entity getViewportEntity() const { return mViewportEntity; };

		/** Creates a new Scene */
		void createScene(const char* name = "");

		/** Destroys the current Scene */
		void destroyScene();

		/** Adds the given panel to the Editor
		 *
		 * @param	panel the new IEditorPanel */
		void addPanel(IEditorPanel* panel);

		/** @copydoc se::app::Application::notify(const IEvent&) */
		virtual bool notify(const se::app::IEvent&) override;
	protected:
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
		 * @param	event the Close to handle */
		void onCloseEvent(const se::app::Event<se::app::Topic::Close>& event);
	};

}

#endif		// EDITOR_H
