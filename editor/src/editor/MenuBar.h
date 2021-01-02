#ifndef MENU_BAR_H
#define MENU_BAR_H

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