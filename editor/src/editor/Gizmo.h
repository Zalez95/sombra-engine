#ifndef GIZMO_H
#define GIZMO_H

namespace editor {

	class Editor;


	/**
	 * Class Gizmo, it's the GUI component used for interacting with an Entity
	 * transforms
	 */
	class Gizmo
	{
	public:		// Nested types
		/** The Operation to perform with the Gizmo */
		enum class Operation { Translation, Rotation, Scale };

	private:	// Attributes
		/** A reference to the Editor that holds the Gizmo */
		Editor& mEditor;

		/** The Operation to perform */
		Operation mOperation;

		/** If the Operation is in world or local space */
		bool mWorld;

	public:		// Functions
		/** Creates a new Gizmo
		 *
		 * @param	editor a reference to the Editor that holds the Entities */
		Gizmo(Editor& editor) :
			mEditor(editor),
			mOperation(Operation::Translation), mWorld(true) {};

		/** Draws the current menu bar */
		void render();
	};
}

#endif		// GIZMO_H
