#ifndef ENTITY_PANEL_H
#define ENTITY_PANEL_H

#include <vector>
#include <se/app/Entity.h>

namespace editor {

	class Editor;


	/**
	 * Class EntityPanel, it's the ImGui panel used for viewing and interacting
	 * with the Scene Entities
	 */
	class EntityPanel
	{
	private:	// Attributes
		/** A reference to the Editor that holds the EntityPanel */
		Editor& mEditor;

	public:		// Functions
		/** Creates a new EntityPanel
		 *
		 * @param	editor a reference to the Editor that holds the MenuBar */
		EntityPanel(Editor& editor) : mEditor(editor) {};

		/** Draws the current panel */
		void render();
	};

}

#endif		// ENTITY_PANEL_H
