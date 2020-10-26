#ifndef ENTITY_PANEL_H
#define ENTITY_PANEL_H

namespace editor {

	class Editor;


	/**
	 * Class EntityPanel, TODO:
	 */
	class EntityPanel
	{
	private:	// Attributes
		/** A reference to the Editor that holds the EntityPanel */
		Editor& mEditor;

		/** If an Entity has been selected or not */
		std::unordered_map<se::app::Entity, bool> mSelectedEntities;

	public:		// Functions
		/** Creates a new EntityPanel
		 *
		 * @param	editor a reference to the Editor that holds the MenuBar */
		EntityPanel(Editor& editor);

		/** Draws the current panel */
		void render();
	private:
		void drawEntities();
		void drawComponents();
	};

}

#endif		// ENTITY_PANEL_H
