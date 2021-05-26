#ifndef ENTITY_PANEL_H
#define ENTITY_PANEL_H

#include "IEditorPanel.h"

namespace editor {

	class Editor;


	/**
	 * Class EntityPanel, it's the Editor panel used for viewing and interacting
	 * with the Scene Entities
	 */
	class EntityPanel : public IEditorPanel
	{
	public:		// Functions
		/** Creates a new EntityPanel
		 *
		 * @param	editor a reference to the Editor that holds the Panel */
		EntityPanel(Editor& editor) : IEditorPanel(editor) {};

		/** @copydoc IEditorPanel::render() */
		virtual bool render() override;
	};

}

#endif		// ENTITY_PANEL_H
