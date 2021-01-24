#ifndef IMGUI_RENDERER_H
#define IMGUI_RENDERER_H

#include <imgui.h>
#include <se/graphics/BindableRenderNode.h>

namespace se::graphics { class VertexArray; }

namespace editor {

	/**
	 * Class ImGuiRenderer. It's a BindableRenderNode used for rendering the
	 * ImGui GUI. It is based on the @see imgui_impl_opengl3.cpp ImGui OpenGL
	 * implementation.
	 */
	class ImGuiRenderer : public se::graphics::BindableRenderNode
	{
	private:	// Nested types
		struct Impl;

	private:	// Values
		Impl* mImpl;

	public:		// Functions
		/** Creates a new ImGuiRenderer
		 *
		 * @param	name the name of the new ImGuiRenderer */
		ImGuiRenderer(const std::string& name);

		/** Class destructor */
		~ImGuiRenderer();

		/** Renders the ImGui GUI */
		virtual void execute() override;
	private:
		bool createDeviceObjects();
		void destroyDeviceObjects();
		bool createFontsTexture();
		void destroyFontsTexture();
		void setupRenderState(
			ImDrawData* drawData, int fbWidth, int fbHeight,
			se::graphics::VertexArray* vao
		);
	};

}

#endif		// IMGUI_RENDERER_H
