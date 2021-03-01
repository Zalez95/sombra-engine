#ifndef COMPONENT_PANEL_H
#define COMPONENT_PANEL_H

#include <vector>
#include <unordered_map>
#include <se/app/Entity.h>

namespace editor {

	class Editor;


	/**
	 * Class ComponentPanel, it's the ImGui panel used for viewing and
	 * interacting the Entity Components
	 */
	class ComponentPanel
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

	private:	// Attributes
		/** A reference to the Editor that holds the ComponentPanel */
		Editor& mEditor;

		/** The objects used for drawing the components */
		std::vector<IComponentNode*> mNodes;

	public:		// Functions
		/** Creates a new ComponentPanel
		 *
		 * @param	editor a reference to the Editor that holds the Entities */
		ComponentPanel(Editor& editor);

		/** Class destructor */
		~ComponentPanel();

		/** Draws the current panel */
		void render();
	};

}

#endif		// COMPONENT_PANEL_H
