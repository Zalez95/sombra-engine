#ifndef GIZMO_H
#define GIZMO_H

#include "IEditorPanel.h"

namespace editor {

	/**
	 * Class Gizmo, it's the Panel used for interacting with an Entity
	 * transforms
	 */
	class Gizmo : public IEditorPanel
	{
	public:		// Nested types
		/** The Operation to perform with the Gizmo */
		enum class Operation { Translation, Rotation, Scale };

	private:	// Attributes
		/** The Operation to perform */
		Operation mOperation = Operation::Translation;

		/** If the Operation is in world or local space */
		bool mWorld = true;

	public:		// Functions
		/** Creates a new Gizmo
		 *
		 * @param	editor a reference to the Editor that holds the Panel */
		Gizmo(Editor& editor) : IEditorPanel(editor) {};

		/** @copydoc IEditorPanel::render() */
		virtual bool render() override;
	};
}

#endif		// GIZMO_H
