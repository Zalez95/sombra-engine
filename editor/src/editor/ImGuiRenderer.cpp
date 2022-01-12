#include <stdio.h>
#include <glm/glm.hpp>
#include <se/utils/Log.h>
#include <se/graphics/core/Shader.h>
#include <se/graphics/core/FrameBuffer.h>
#include <se/graphics/core/VertexArray.h>
#include <se/graphics/core/IndexBuffer.h>
#include <se/graphics/core/VertexBuffer.h>
#include <se/graphics/core/UniformVariable.h>
#include <se/graphics/core/GraphicsOperations.h>
#include <se/app/graphics/TypeRefs.h>
#include "ImGuiRenderer.h"

using namespace se::app;
using namespace se::graphics;
using namespace std::string_literals;

namespace editor {

	struct ImGuiRenderer::Impl
	{
		TextureRef fontTexture;
		ProgramRef program;
		UniformVVRef<int> uTextureUniform;
		UniformVVRef<glm::mat4> uProjectionMatrix;
		Context::TBindableRef<VertexBuffer> vbo;
		Context::TBindableRef<IndexBuffer> ibo;

		static constexpr int kDrawTextureUnit = 0;
	};


	ImGuiRenderer::ImGuiRenderer(const std::string& name, Context& context) : BindableRenderNode(name), mImpl(nullptr)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.BackendRendererName = getName().c_str();

		auto iTargetBindable = addBindable();
		addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("target", this, iTargetBindable) );
		addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("target", this, iTargetBindable) );

		if (!mImpl) {
			createDeviceObjects(context);
		}
	}


	ImGuiRenderer::~ImGuiRenderer()
	{
		if (mImpl) {
			destroyDeviceObjects();
		}
	}


	void ImGuiRenderer::execute(Context::Query& q)
	{
		ImGui::Render();
		bind(q);

		// Store the current GL state
		int lastX, lastY;
		std::size_t lastWidth, lastHeight;
		GraphicsOperations::getViewport(lastX, lastY, lastWidth, lastHeight);

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
		SetOperation opBlending(Operation::Blending, true);			opBlending.bind();
		SetOperation opCulling(Operation::Culling, false);			opCulling.bind();
		SetOperation opDepthTest(Operation::DepthTest, false);		opDepthTest.bind();
		SetOperation opScissorTest(Operation::ScissorTest, true);	opScissorTest.bind();

		VertexArray vao;
		setupRenderState(drawData, fbWidth, fbHeight, q, &vao);

		// Will project scissor/clipping rectangles into framebuffer space
		ImVec2 clipOff = drawData->DisplayPos;         // (0,0) unless using multi-viewports
		ImVec2 clipScale = drawData->FramebufferScale; // (1,1) unless using retina display which are often (2,2)

		// Render command lists
		for (int n = 0; n < drawData->CmdListsCount; ++n) {
			const ImDrawList* cmdList = drawData->CmdLists[n];

			// Upload vertex/index buffers
			q.getTBindable(mImpl->vbo)->resizeAndCopy(cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size);
			q.getTBindable(mImpl->ibo)->resizeAndCopy(cmdList->IdxBuffer.Data, (sizeof(ImDrawIdx) == 2)? TypeId::UnsignedShort : TypeId::UnsignedInt, cmdList->IdxBuffer.Size);

			for (int iCmd = 0; iCmd < cmdList->CmdBuffer.Size; ++iCmd) {
				const ImDrawCmd* pcmd = &cmdList->CmdBuffer[iCmd];
				if (pcmd->UserCallback != nullptr) {
					// User callback, registered via ImDrawList::AddCallback()
					// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
					if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
						setupRenderState(drawData, fbWidth, fbHeight, q, &vao);
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
						GraphicsOperations::setScissorBox(
							static_cast<int>(clipRect.x), static_cast<int>(fbHeight - clipRect.w),
							static_cast<std::size_t>(clipRect.z - clipRect.x), static_cast<std::size_t>(clipRect.w - clipRect.y)
						);

						// Bind atlas texture
						if (pcmd->TextureId) {
							Texture* texture = static_cast<Texture*>(pcmd->TextureId);
							texture->bind();
						}

						// Draw
						GraphicsOperations::drawIndexed(PrimitiveType::Triangle, pcmd->ElemCount,
							(sizeof(ImDrawIdx) == 2)? TypeId::UnsignedShort : TypeId::UnsignedInt,
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
		GraphicsOperations::setViewport(lastX, lastY, lastWidth, lastHeight);
	}

// Private functions
	bool ImGuiRenderer::createDeviceObjects(Context& context)
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
			mImpl->program = context.create<Program>()
				.edit([=](Program& program) {
					Shader vertexShader(vertexShaderStr, ShaderType::Vertex);
					Shader fragmentShader(fragmentShaderStr, ShaderType::Fragment);
					Shader* shaderPtrs[] = { &vertexShader, &fragmentShader };
					program.load(shaderPtrs, 2);
				});

			mImpl->uTextureUniform = context.create<UniformVariableValue<int>>("Texture"s)
				.qedit([pRef = mImpl->program](auto& q, auto& uniform) {
					uniform.load(*q.getTBindable(pRef));
					uniform.setValue(Impl::kDrawTextureUnit);
				});
			mImpl->uProjectionMatrix = context.create<UniformVariableValue<glm::mat4>>("ProjMtx"s)
				.qedit([pRef = mImpl->program](auto& q, auto& uniform) {
					uniform.load(*q.getTBindable(pRef));
				});

			// Create buffers
			mImpl->vbo = context.create<VertexBuffer>();
			mImpl->ibo = context.create<IndexBuffer>();

			ret = createFontsTexture(context);
		}
		catch (std::exception& e) {
			SOMBRA_FATAL_LOG << "Error while creating the ImGui Programs: " << e.what();
			ret = false;
		}

		return ret;
	}


	void ImGuiRenderer::destroyDeviceObjects()
	{
		destroyFontsTexture();

		delete mImpl;
		mImpl = nullptr;
	}


	bool ImGuiRenderer::createFontsTexture(Context& context)
	{
		// Build texture atlas
		ImGuiIO& io = ImGui::GetIO();
		unsigned char* pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

		// Upload texture to graphics system
		mImpl->fontTexture = context.create<Texture>(TextureTarget::Texture2D);
		mImpl->fontTexture.edit([=](Texture& tex) {
			tex.setImage(pixels, TypeId::UnsignedByte, ColorFormat::RGBA, ColorFormat::RGBA, width, height)
				.setFiltering(TextureFilter::Linear, TextureFilter::Linear)
				.setTextureUnit(Impl::kDrawTextureUnit);

			ImGuiIO& io = ImGui::GetIO();
			io.Fonts->TexID = static_cast<ImTextureID>(&tex);
		});

		return true;
	}


	void ImGuiRenderer::destroyFontsTexture()
	{
		if (mImpl->fontTexture) {
			ImGuiIO& io = ImGui::GetIO();
			io.Fonts->TexID = 0;
			mImpl->fontTexture = {};
		}
	}


	void ImGuiRenderer::setupRenderState(ImDrawData* drawData, int fbWidth, int fbHeight, Context::Query& q, VertexArray* vao)
	{
		// Support for GL 4.5 rarely used glClipControl(GL_UPPER_LEFT)
		bool clip_origin_lower_left = true;

		// Setup viewport, orthographic projection matrix
		// Our visible imgui space lies from drawData->DisplayPos (top left) to drawData->DisplayPos+data_data->DisplaySize (bottom right). DisplayPos is (0,0) for single viewport apps.
		GraphicsOperations::setViewport(0, 0, fbWidth, fbHeight);
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
		q.getBindable(mImpl->program)->bind();
		q.getTBindable(mImpl->uProjectionMatrix)->setValue(ortho_projection);
		q.getTBindable(mImpl->uProjectionMatrix)->bind();

		// Bind vertex/index buffers and setup attributes for ImDrawVert
		vao->bind();
		q.getTBindable(mImpl->vbo)->bind();
		q.getTBindable(mImpl->ibo)->bind();

		vao->enableAttribute(0);
		vao->setVertexAttribute(0, TypeId::Float, false, 2, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, pos));
		vao->enableAttribute(1);
		vao->setVertexAttribute(1, TypeId::Float, false, 2, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, uv));
		vao->enableAttribute(2);
		vao->setVertexAttribute(2, TypeId::UnsignedByte, true, 4, sizeof(ImDrawVert), IM_OFFSETOF(ImDrawVert, col));
	}

}
