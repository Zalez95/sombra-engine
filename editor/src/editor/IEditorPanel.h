#ifndef IEDITOR_PANEL_H
#define IEDITOR_PANEL_H

namespace editor {

	class Editor;


	/**
	 * Class IEditorPanel, it's a drawable Panel of the Editor
	 */
	class IEditorPanel
	{
	protected:	// Attributes
		/** A reference to the Editor that holds the IEditorPanel */
		Editor& mEditor;

		/** The id of the Panel */
		int mPanelId = -1;

	public:		// Functions
		/** Creates a new IEditorPanel
		 *
		 * @param	editor a reference to the Editor that holds the Panel */
		IEditorPanel(Editor& editor) : mEditor(editor) {};

		/** Class destructor */
		virtual ~IEditorPanel() = default;

		/** @return	the id of the Panel */
		int getId() const { return mPanelId; };

		/** Sets the id of the Panel
		 *
		 * @param	id the new Id of the Panel */
		void setId(int id) { mPanelId = id; };

		/** Draws the current panel
		 *
		 * @return	true if the IEditorPanel is active, false if it's closed */
		virtual bool render() { return false; };
	};

}

#endif		// IEDITOR_PANEL_H
