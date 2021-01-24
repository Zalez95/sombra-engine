#ifndef REPOSITORY_PANEL_H
#define REPOSITORY_PANEL_H

namespace editor {

	class Editor;


	/**
	 * Class EntityPanel, it's the ImGui panel used for viewing and interacting
	 * with the Scene resources added to its Repository
	 */
	class RepositoryPanel
	{
	private:	// Nested types
		class ITypeNode;
		template <typename T> class TypeNode;
		template <typename T> class ImportTypeNode;
		template <typename T> class SceneReaderTypeNode;
		class SkinNode;
		class LightSourceNode;
		class RenderableShaderNode;
		class CompositeAnimatorNode;
		class AudioBufferNode;
		class ForceNode;
		class ProgramNode;
		class PassNode;
		class TechniqueNode;
		class TextureNode;
		class MeshNode;

	private:	// Attributes
		/** A reference to the Editor that holds the RepositoryPanel */
		Editor& mEditor;

		/** The types that can be viewed */
		std::vector<ITypeNode*> mTypes;

		/** The current type to be viewed */
		int mTypeSelected;

	public:		// Functions
		/** Creates a new RepositoryPanel
		 *
		 * @param	editor a reference to the Editor that holds the MenuBar */
		RepositoryPanel(Editor& editor);

		/** Class destructor */
		~RepositoryPanel();

		/** Draws the current panel */
		void render();
	};

}

#endif		// REPOSITORY_PANEL_H
