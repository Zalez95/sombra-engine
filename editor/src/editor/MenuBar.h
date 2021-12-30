#ifndef MENU_BAR_H
#define MENU_BAR_H

#include <future>
#include "se/app/io/Result.h"
#include "ImGuiUtils.h"

namespace editor {

	class Editor;


	/**
	 * Class MenuBar. Its holds all the functionality of the top menu bar of
	 * the Editor
	 */
	class MenuBar
	{
	private:	// Attributes
		/** A reference to the Editor that holds the MenuBar */
		Editor& mEditor;

		/** The window used for opening/importing/saving files */
		FileWindow mWindow;

		/** If an operation should be cancelled or not */
		bool mCancel = false;

		/** The error string, empty if the error popup must not be shown */
		std::string mError;

		/** If the current scene must be closed or not */
		bool mClose = false;

		/** If a new scene must be created or not */
		bool mCreate = false;

		/** @see mWindow is being used for opening a Scene */
		bool mOpen = false;

		/** @see mWindow is being used for appending a Scene */
		bool mAppend = false;

		/** @see mWindow is being used for linking a Scene */
		bool mLink = false;

		/** @see mWindow is being used for importing a Scene */
		bool mImport = false;

		/** @see mWindow is being used for saving a Scene */
		bool mSave = false;

		/** If the Controls window should be shown or not */
		bool mShowControlsWindow = false;

		/** If the About window should be shown or not */
		bool mShowAboutWindow = false;

		/** The future result of the operations */
		std::future<se::app::Result> mFutureResult;

	public:		// Functions
		/** Creates a new MenuBar
		 *
		 * @param	editor a reference to the Editor that holds the MenuBar */
		MenuBar(Editor& editor) : mEditor(editor), mWindow("MenuBar") {};

		/** Draws the current menu bar */
		void render();

		/** Error handling */
		void doError();

		/** Closes the current Scene if the user want's to */
		void closeScene();

		/** Creates a new Scene, closing the current one if the user want's
		 * to */
		void createScene();

		/** Window handling */
		void doWindow();

		/** Shows the Controls window */
		void showControls();

		/** Shows the About window */
		void showAbout();
	};

}

#endif		// MENU_BAR_H
