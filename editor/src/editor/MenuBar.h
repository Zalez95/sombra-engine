#ifndef MENU_BAR_H
#define MENU_BAR_H

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

		/** @see mWindow is being used for opening a Scene */
		bool mOpen;

		/** @see mWindow is being used for importing a Scene */
		bool mImport;

		/** @see mWindow is being used for saving a Scene */
		bool mSave;

	public:		// Functions
		/** Creates a new MenuBar
		 *
		 * @param	editor a reference to the Editor that holds the MenuBar */
		MenuBar(Editor& editor);

		/** Draws the current menu bar */
		void render();
	};

}

#endif		// MENU_BAR_H
