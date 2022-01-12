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
		 * @param	name the name of the new ImGuiRenderer
		 * @param	context the graphics Context used for creating the
		 *			Bindables */
		ImGuiRenderer(const std::string& name, se::graphics::Context& context);

		/** Class destructor */
		~ImGuiRenderer();

		/** @copydoc se::graphics::RenderNode::execute(se::graphics::Context::Query&) */
		virtual void execute(se::graphics::Context::Query& q) override;
	private:
		bool createDeviceObjects(se::graphics::Context& context);
		void destroyDeviceObjects();
		bool createFontsTexture(se::graphics::Context& context);
		void destroyFontsTexture();
		void setupRenderState(
			ImDrawData* drawData, int fbWidth, int fbHeight,
			se::graphics::Context::Query& q, se::graphics::VertexArray* vao
		);
	};

}

#endif		// IMGUI_RENDERER_H
