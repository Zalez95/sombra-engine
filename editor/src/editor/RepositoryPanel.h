#ifndef REPOSITORY_PANEL_H
#define REPOSITORY_PANEL_H

#include "IEditorPanel.h"

namespace editor {

	/**
	 * Class EntityPanel, it's the ImGui panel used for viewing and interacting
	 * with the Scene resources added to its Repository
	 */
	class RepositoryPanel : public IEditorPanel
	{
	private:	// Nested types
		class ITypeNode;
		template <typename T> class TypeNode;
		template <typename T> class ImportTypeNode;
		template <typename T> class SceneImporterTypeNode;
		class SkinNode;
		class LightSourceNode;
		class SkeletonAnimatorNode;
		class AudioBufferNode;
		class ForceNode;
		class ProgramNode;
		class RenderableShaderStepNode;
		class RenderableShaderNode;
		class TextureNode;
		class MeshNode;
		class ParticleEmitterNode;

	private:	// Attributes
		/** The types that can be viewed */
		std::vector<ITypeNode*> mTypes;

		/** The current type to be viewed */
		int mTypeSelected = -1;

	public:		// Functions
		/** Creates a new RepositoryPanel
		 *
		 * @param	editor a reference to the Editor that holds the Panel */
		RepositoryPanel(Editor& editor);

		/** Class destructor */
		~RepositoryPanel();

		/** @copydoc IEditorPanel::render() */
		virtual bool render() override;
	};

}

#endif		// REPOSITORY_PANEL_H
