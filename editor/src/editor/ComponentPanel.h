#ifndef COMPONENT_PANEL_H
#define COMPONENT_PANEL_H

#include <vector>
#include <unordered_map>
#include "IEditorPanel.h"

namespace editor {

	/**
	 * Class ComponentPanel, it's the ImGui panel used for viewing and
	 * interacting the Entity Components
	 */
	class ComponentPanel : public IEditorPanel
	{
	private:	// Nested types
		class IComponentNode;
		template <typename T> class ComponentNode;
		class TagComponentNode;
		class TransformsComponentNode;
		class AnimationComponentNode;
		class CameraComponentNode;
		class LightComponentNode;
		class LightProbeComponentNode;
		class MeshComponentNode;
		class TerrainComponentNode;
		class RigidBodyComponentNode;
		class ColliderComponentNode;
		class ParticleSystemComponentNode;
		class AudioSourceComponentNode;

	private:	// Attributes
		/** The objects used for drawing the components */
		std::vector<IComponentNode*> mNodes;

	public:		// Functions
		/** Creates a new ComponentPanel
		 *
		 * @param	editor a reference to the Editor that holds the Panel */
		ComponentPanel(Editor& editor);

		/** Class destructor */
		~ComponentPanel();

		/** @copydoc IEditorPanel::render() */
		virtual bool render() override;
	};

}

#endif		// COMPONENT_PANEL_H
