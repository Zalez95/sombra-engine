#include <stdio.h>
#include <glm/glm.hpp>
#include <se/utils/Log.h>
#include <se/graphics/core/Shader.h>
#include <se/graphics/core/Program.h>
#include <se/graphics/core/Texture.h>
#include <se/graphics/core/FrameBuffer.h>
#include <se/graphics/core/VertexArray.h>
#include <se/graphics/core/IndexBuffer.h>
#include <se/graphics/core/VertexBuffer.h>
#include <se/graphics/core/UniformVariable.h>
#include <se/graphics/core/GraphicsOperations.h>
#include "ImGuiRenderer.h"

namespace editor {

	struct ImGuiRenderer::Impl
	{
		se::graphics::Texture* fontTexture = nullptr;
		se::graphics::Program* program = nullptr;
		se::graphics::UniformVariableValue<int>* uTextureUniform = nullptr;
		se::graphics::UniformVariableValue<glm::mat4>* uProjectionMatrix = nullptr;
		se::graphics::VertexBuffer* vbo = nullptr;
		se::graphics::IndexBuffer* ibo = nullptr;
	};


	ImGuiRenderer::ImGuiRenderer(const std::string& name) : se::graphics::BindableRenderNode(name), mImpl(nullptr)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.BackendRendererName = getName().c_str();

		auto iTargetBindable = addBindable();
		addInput( std::make_unique<se::graphics::BindableRNodeInput<se::graphics::FrameBuffer>>("target", this, iTargetBindable) );
		addOutput( std::make_unique<se::graphics::BindableRNodeOutput<se::graphics::FrameBuffer>>("target", this, iTargetBindable) );

		if (!mImpl) {
			createDeviceObjects();
		}
	}


	ImGuiRenderer::~ImGuiRenderer()
	{
		if (mImpl) {
			destroyDeviceObjects();
		}
	}


	void ImGuiRenderer::execute()
	{
		ImGui::Render();
		bind();

		// Store the current GL state
		int lastX, lastY;
		std::size_t lastWidth, lastHeight;
		se::graphics::GraphicsOperations::getViewport(lastX, lastY, lastWidth, lastHeight);

		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		ImDrawData* drawData = ImGui::GetDrawData();
		int fbWidth = static_cast<int>(drawData->DisplaySize.x * drawData->FramebufferScale.x);
		int fbHeight = static_cast<int>(drawData->DisplaySize.y * drawData->FramebufferScale.y);
		if (fbWidth <= 0 || fbHeight <= 0) {
			return;
		}

		// Setup desired GL state
		// Recreate the VAO every time (this is to easily allow multiple GL contexts to be rendered to. VAO are not shared among GL contexts)
		// The renderer would actually work without any VAO bound, but then our VertexAttrib calls would overwrite the default one currently bound.
		se::graphics::SetOperation opBlending(se::graphics::Operation::Blending, true);			opBlending.bind();
		se::graphics::SetOperation opCulling(se::graphics::Operation::Culling, false);			opCulling.bind();
		se::graphics::SetOperation opDepthTest(se::graphics::Operation::DepthTest, false);		opDepthTest.bind();
		se::graphics::SetOperation opScissorTest(se::graphics::Operation::ScissorTest, true);	opScissorTest.bind();

		se::graphics::VertexArray vao;
		setupRenderState(drawData, fbWidth, fbHeight, &vao);

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clipOff = drawData->DisplayPos;         // (0,0) unless using multi-viewports
		ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		for (int n = 0; n < drawData->CmdListsCount; ++n) {
			const ImDrawList* cmdList = drawData->CmdLists[n];

			// Upload vertex/index buffers
			mImpl->vbo->resizeAndCopy(cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size);
			mImpl->ibo->resizeAndCopy(cmdList->IdxBuffer.Data, (sizeof(ImDrawIdx) == 2)? se::graphics::TypeId::UnsignedShort : se::graphics::TypeId::UnsignedInt, cmdList->IdxBuffer.Size);

			for (int iCmd = 0; iCmd < cmdList->CmdBuffer.Size; ++iCmd) {
				const ImDrawCmd* pcmd = &cmdList->CmdBuffer[iCmd];
				if (pcmd->UserCallback != nullptr) {
					// User callback, registered via ImDrawList::AddCallback()
					// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
					if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
						setupRenderState(drawData, fbWidth, fbHeight, &vao);
					}
					else {
						pcmd->UserCallback(cmdList, pcmd);
					}
				}
				else {
					// Project scissor/clipping rectangles into framebuffer space
					ImVec4 clipRect;
					clipRect.x = (pcmd->ClipRect.x - clipOff.x) * clipScale.x;
					clipRect.y = (pcmd->ClipRect.y - clipOff.y) * clipScale.y;
					clipRect.z = (pcmd->ClipRect.z - clipOff.x) * clipScale.x;
					clipRect.w = (pcmd->ClipRect.w - clipOff.y) * clipScale.y;

					if (clipRect.x < fbWidth && clipRect.y < fbHeight && clipRect.z >= 0.0f && clipRect.w >= 0.0f) {
						// Apply scissor/clipping rectangle
						se::graphics::GraphicsOperations::setScissorBox(
							static_cast<int>(clipRect.x), static_cast<int>(fbHeight - clipRect.w),
							static_cast<std::size_t>(clipRect.z - clipRect.x), static_cast<std::size_t>(clipRect.w - clipRect.y)
						);

						// Bind texture, Draw
						se::graphics::Texture* texture = static_cast<se::graphics::Texture*>(pcmd->TextureId);
						texture->setTextureUnit(0);
						texture->bind();
						se::graphics::GraphicsOperations::drawIndexed(se::graphics::PrimitiveType::Triangle, pcmd->ElemCount,
							(sizeof(ImDrawIdx) == 2)? se::graphics::TypeId::UnsignedShort : se::graphics::TypeId::UnsignedInt,
							pcmd->IdxOffset * sizeof(ImDrawIdx)
						);
					}
				}
			}
		}

		// Restore modified GL state
		opScissorTest.unbind();
		opDepthTest.unbind();
		opCulling.unbind();
		opBlending.unbind();
		se::graphics::GraphicsOperations::setViewport(lastX, lastY, lastWidth, lastHeight);
	}

// Private functions
	bool ImGuiRenderer::createDeviceObjects()
	{
		bool ret = true;

		mImpl = new Impl();

		const char* vertexShaderStr =
			"#version 330 core\n"
			"layout (location = 0) in vec2 Position;\n"
			"layout (location = 1) in vec2 UV;\n"
			"layout (location = 2) in vec4 Color;\n"
			"uniform mat4 ProjMtx;\n"
			"out vec2 Frag_UV;\n"
			"out vec4 Frag_Color;\n"
			"void main()\n"
			"{\n"
			"    Frag_UV = UV;\n"
			"    Frag_Color = Color;\n"
			"    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
			"}\n";

		const char* fragmentShaderStr =
			"#version 330 core\n"
			"uniform sampler2D Texture;\n"
			"in vec2 Frag_UV;\n"
			"in vec4 Frag_Color;\n"
			"out vec4 Out_Color;\n"
			"void main()\n"
			"{\n"
			"    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
			"}\n";

		try {
			// Create shaders
			se::graphics::Shader vertexShader(vertexShaderStr, se::graphics::ShaderType::Vertex);
			se::graphics::Shader fragmentShader(fragmentShaderStr, se::graphics::ShaderType::Fragment);
			se::graphics::Shader* shaders[] = { &vertexShader, &fragmentShader };
			mImpl->program = new se::graphics::Program(shaders, 2);

			mImpl->uTextureUniform = new se::graphics::UniformVariableValue<int>("Texture", *mImpl->program);
			mImpl->uProjectionMatrix = new se::graphics::UniformVariableValue<glm::mat4>("ProjMtx", *mImpl->program);

			// Create buffers
			mImpl->vbo = new se::graphics::VertexBuffer();
			mImpl->ibo = new se::graphics::IndexBuffer();

			ret = createFontsTexture();
		}
		catch (std::exception& e) {
			SOMBRA_FATAL_LOG << "Error while creating the ImGui Programs: " << e.what();
			ret = false;
		}

		return ret;
	}


	void ImGuiRenderer::destroyDeviceObjects()
	{
		if (mImpl->vbo) { delete mImpl->vbo; }
		if (mImpl->ibo) { delete mImpl->ibo; }
		if (mImpl->uTextureUniform) { delete mImpl->uTextureUniform; }
		if (mImpl->uProjectionMatrix) { delete mImpl->uProjectionMatrix; }
		if (mImpl->program) { delete mImpl->program; }

		destroyFontsTexture();

		delete mImpl;
		mImpl = nullptr;
	}


	bool ImGuiRenderer::createFontsTexture()
	{
		// Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

		// Upload texture to graphics system
		mImpl->fontTexture = new se::graphics::Texture(se::graphics::TextureTarget::Texture2D);
		mImpl->fontTexture->setImage(pixels, se::graphics::TypeId::UnsignedByte, se::graphics::ColorFormat::RGBA, se::graphics::ColorFormat::RGBA, width, height);
		mImpl->fontTexture->setFiltering(se::graphics::TextureFilter::Linear, se::graphics::TextureFilter::Linear);
		mImpl->fontTexture->setTextureUnit(0);

		io.Fonts->TexID = static_cast<ImTextureID>(mImpl->fontTexture);

		return true;
	}


	void ImGuiRenderer::destroyFontsTexture()
	{
		if (mImpl->fontTexture) {
			ImGuiIO& io = ImGui::GetIO();
			io.Fonts->TexID = 0;
			delete mImpl->fontTexture;
		}
	}


	void ImGuiRenderer::setupRenderState(ImDrawData* drawData, int fbWidth, int fbHeight, se::graphics::VertexArray* vao)
	{
		// Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
		bool clip_origin_lower_left = true;

		// Setup viewport, orthographic projection matrix
		// Our visible imgui space lies from drawData->DisplayPos (top left) to drawData->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
		se::graphics::GraphicsOperations::setViewport(0, 0, fbWidth, fbHeight);
		float L = drawData->DisplayPos.x;
		float R = drawData->DisplayPos.x + drawData->DisplaySize.x;
		float T = drawData->DisplayPos.y;
		float B = drawData->DisplayPos.y + drawData->DisplaySize.y;
		if (!clip_origin_lower_left) { float tmp = T; T = B; B = tmp; } // Swap top and bottom if origin is upper left
		const glm::mat4 ortho_projection = {
			{ 2.0f/(R-L),   0.0f,         0.0f,   0.0f },
			{ 0.0f,         2.0f/(T-B),   0.0f,   0.0f },
			{ 0.0f,         0.0f,        -1.0f,   0.0f },
			{ (R+L)/(L-R),  (T+B)/(B-T),  0.0f,   1.0f },
		};
		mImpl->program->bind();
		mImpl->uTextureUniform->setValue(0);
		mImpl->uProjectionMatrix->setValue(ortho_projection);
		mImpl->uTextureUniform->bind();
		mImpl->uProjectionMatrix->bind();

		// Bind vertex/index buffers and setup attributes for ImDrawVert
		vao->bind();
		mImpl->vbo->bind();
		mImpl->ibo->bind();

		vao->setVertexAttribute(0, se::graphics::TypeId::Float, false, 2, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, pos));
		vao->setVertexAttribute(1, se::graphics::TypeId::Float, false, 2, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, uv));
		vao->setVertexAttribute(2, se::graphics::TypeId::UnsignedByte, true, 4, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, col));
	}

}
