#include <glm/gtc/matrix_transform.hpp>
#include "se/app/graphics/TextureUtils.h"
#include "se/app/io/MeshLoader.h"
#include "se/app/io/ShaderLoader.h"
#include "se/graphics/Technique.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/3D/RenderableMesh.h"
#include "se/graphics/FBClearNode.h"
#include "se/graphics/3D/RendererMesh.h"

namespace se::app {

	static const int kMaxMipPrefilterLevels = 5;
	static const glm::mat4 kCubeMapViewMatrices[] = {
		glm::lookAt(glm::vec3(0.0f), { 1.0f, 0.0f, 0.0f }, { 0.0f,-1.0f, 0.0f }),
		glm::lookAt(glm::vec3(0.0f), {-1.0f, 0.0f, 0.0f }, { 0.0f,-1.0f, 0.0f }),
		glm::lookAt(glm::vec3(0.0f), { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }),
		glm::lookAt(glm::vec3(0.0f), { 0.0f,-1.0f, 0.0f }, { 0.0f, 0.0f,-1.0f }),
		glm::lookAt(glm::vec3(0.0f), { 0.0f, 0.0f, 1.0f }, { 0.0f,-1.0f, 0.0f }),
		glm::lookAt(glm::vec3(0.0f), { 0.0f, 0.0f,-1.0f }, { 0.0f,-1.0f, 0.0f }),
	};
	static const glm::mat4 kCubeMapProjectionMatrix = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);


	class MyRenderGraph : public graphics::RenderGraph
	{
	public:		// Functions
		MyRenderGraph(graphics::Context& context) : graphics::RenderGraph(context)
		{
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(getNode("resources"));
			auto frameBufferIndex = resources->addBindable( context.create<graphics::FrameBuffer>() );
			resources->addOutput(std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("frameBuffer", resources, frameBufferIndex));

			auto clearMask = graphics::FrameBufferMask::Mask().set(graphics::FrameBufferMask::kColor).set(graphics::FrameBufferMask::kDepth);
			auto fbClearNode = std::make_unique<graphics::FBClearNode>("fbClearNode", clearMask);

			auto rendererMesh = std::make_unique<graphics::RendererMesh>("rendererMesh");

			fbClearNode->findInput("target")->connect(resources->findOutput("frameBuffer"));
			rendererMesh->findInput("target")->connect(fbClearNode->findOutput("target"));
			addNode(std::move(rendererMesh));
			addNode(std::move(fbClearNode));

			prepareGraph();
		};
	};


	TextureRef TextureUtils::equirectangularToCubeMap(const TextureRef& source, std::size_t size)
	{
		graphics::Context& context = *source.getParent();

		// Create the CubeMap
		auto ret = context.create<graphics::Texture>(graphics::TextureTarget::CubeMap);
		ret.edit([=](graphics::Texture& tex) {
			for (int i = 0; i < 6; ++i) {
				tex.setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB16f, size, size, 0, i);
			}
			tex.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::LinearMipMapLinear, graphics::TextureFilter::Linear);
		});

		// Create the RenderGraph
		auto graph = std::make_shared<MyRenderGraph>(context);
		auto resources = graph->getNode("resources");
		auto frameBuffer = dynamic_cast<graphics::BindableRNodeOutput<graphics::FrameBuffer>*>(resources->findOutput("frameBuffer"))->getTBindable();

		auto depthTexture = context.create<graphics::Texture>(graphics::TextureTarget::Texture2D);
		depthTexture.edit([=](graphics::Texture& tex) {
			tex.setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, size, size);
		});
		frameBuffer.getParent()->execute([=](graphics::Context::Query& q) {
			q.getTBindable(frameBuffer)->attach(q.getTBindable(depthTexture), graphics::FrameBufferAttachment::kDepth);
		});

		// Create the Technique
		ProgramRef program;
		auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentEquiToCubeMap.glsl", context, program);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return {};
		}

		source.edit([](graphics::Texture& tex) { tex.setTextureUnit(0); });

		auto pass = std::make_shared<graphics::Pass>( *dynamic_cast<graphics::Renderer*>(graph->getNode("rendererMesh")) );
		auto viewMatrixUniform = context.create<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix")
			.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); });
		pass->addBindable(program)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", glm::mat4(1.0f))
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			)
			.addBindable(viewMatrixUniform)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", kCubeMapProjectionMatrix)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			)
			.addBindable(source)
			.addBindable(
				context.create<graphics::UniformVariableValue<int>>("uEquirectangularMap", 0)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			);
		auto technique = std::make_shared<graphics::Technique>();
		technique->addPass(pass);

		// Create the Cube RenderableMesh
		auto cubeRawMesh = MeshLoader::createBoxMesh("cube", glm::vec3(1.0f));
		auto cubeMesh = MeshLoader::createGraphicsMesh(context, cubeRawMesh);
		auto cubeRenderable = std::make_shared<graphics::RenderableMesh>(std::move(cubeMesh));
		cubeRenderable->addTechnique(technique);

		// Render the Environment Map to each of the Faces of the CubeMap
		for (int i = 0; i < 6; ++i) {
			frameBuffer.getParent()->execute([=](graphics::Context::Query& q) {
				q.getTBindable(frameBuffer)->attach(q.getTBindable(ret), graphics::FrameBufferAttachment::kColor0, 0, 0, i);
			});
			viewMatrixUniform.edit([=](graphics::UniformVariableValue<glm::mat4>& uniform) {
				uniform.setValue(kCubeMapViewMatrices[i]);
			});

			context.execute([=](graphics::Context::Query& q) {
				int originX, originY;
				std::size_t dimensionsX, dimensionsY;
				graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
				graphics::GraphicsOperations::setViewport(0, 0, size, size);

				cubeRenderable->submit(q);
				graph->execute(q);

				graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);
			});
		}

		ret.edit([](graphics::Texture& tex) { tex.generateMipMap(); });

		return ret;
	}


	TextureRef TextureUtils::convoluteCubeMap(const TextureRef& source, std::size_t size)
	{
		graphics::Context& context = *source.getParent();

		// Create the CubeMap
		auto ret = context.create<graphics::Texture>(graphics::TextureTarget::CubeMap);
		ret.edit([=](graphics::Texture& tex) {
			for (int i = 0; i < 6; ++i) {
				tex.setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB16f, size, size, 0, i);
			}
			tex.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge);
		});

		// Create the RenderGraph
		auto graph = std::make_shared<MyRenderGraph>(*source.getParent());
		auto resources = graph->getNode("resources");
		auto frameBuffer = dynamic_cast<graphics::BindableRNodeOutput<graphics::FrameBuffer>*>(resources->findOutput("frameBuffer"))->getTBindable();

		auto depthTexture = context.create<graphics::Texture>(graphics::TextureTarget::Texture2D);
		depthTexture.edit([=](graphics::Texture& tex) {
			tex.setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, size, size);
		});
		frameBuffer.getParent()->execute([=](graphics::Context::Query& q) {
			q.getTBindable(frameBuffer)->attach(q.getTBindable(depthTexture), graphics::FrameBufferAttachment::kDepth);
		});

		// Create the Technique
		ProgramRef program;
		auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentConvoluteCubeMap.glsl", context, program);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return {};
		}

		source.edit([](graphics::Texture& tex) { tex.setTextureUnit(0); });

		auto pass = std::make_shared<graphics::Pass>( *dynamic_cast<graphics::Renderer*>(graph->getNode("rendererMesh")) );
		auto viewMatrixUniform = context.create<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix")
			.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); });
		pass->addBindable(program)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", glm::mat4(1.0f))
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			)
			.addBindable(viewMatrixUniform)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", kCubeMapProjectionMatrix)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			)
			.addBindable(source)
			.addBindable(
				context.create<graphics::UniformVariableValue<int>>("uCubeMap", 0)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			);
		auto technique = std::make_shared<graphics::Technique>();
		technique->addPass(pass);

		// Create the Cube RenderableMesh
		auto cubeRawMesh = MeshLoader::createBoxMesh("cube", glm::vec3(1.0f));
		auto cubeMesh = MeshLoader::createGraphicsMesh(context, cubeRawMesh);
		auto cubeRenderable = std::make_shared<graphics::RenderableMesh>(std::move(cubeMesh));
		cubeRenderable->addTechnique(technique);

		// Render the Environment Map to each of the Faces of the CubeMap
		for (int i = 0; i < 6; ++i) {
			frameBuffer.qedit([=](auto& q, auto& fb) {
				fb.attach(q.getTBindable(ret), graphics::FrameBufferAttachment::kColor0, 0, 0, i);
			});
			viewMatrixUniform.edit([=](graphics::UniformVariableValue<glm::mat4>& uniform) {
				uniform.setValue(kCubeMapViewMatrices[i]);
			});

			context.execute([=](graphics::Context::Query& q) {
				int originX, originY;
				std::size_t dimensionsX, dimensionsY;
				graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
				graphics::GraphicsOperations::setViewport(0, 0, size, size);

				cubeRenderable->submit(q);
				graph->execute(q);

				graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);
			});
		}

		return ret;
	}


	TextureRef TextureUtils::prefilterCubeMap(const TextureRef& source, std::size_t size)
	{
		graphics::Context& context = *source.getParent();

		// Create the CubeMap
		auto ret = context.create<graphics::Texture>(graphics::TextureTarget::CubeMap);
		ret.edit([=](graphics::Texture& tex) {
			for (int i = 0; i < 6; ++i) {
				tex.setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB16f, size, size, 0, i);
			}
			tex.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::LinearMipMapLinear, graphics::TextureFilter::Linear)
				.generateMipMap();
		});

		// Create the RenderGraph
		auto graph = std::make_shared<MyRenderGraph>(*source.getParent());
		auto resources = graph->getNode("resources");
		auto frameBuffer = dynamic_cast<graphics::BindableRNodeOutput<graphics::FrameBuffer>*>(resources->findOutput("frameBuffer"))->getTBindable();

		auto depthTexture = context.create<graphics::Texture>(graphics::TextureTarget::Texture2D);
		depthTexture.edit([=](graphics::Texture& tex) {
			tex.setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, size, size)
				.generateMipMap();
		});

		// Create the Technique
		ProgramRef program;
		auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentPrefilterCubeMap.glsl", context, program);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return {};
		}

		source.edit([](graphics::Texture& tex) { tex.setTextureUnit(0); });

		auto pass = std::make_shared<graphics::Pass>( *dynamic_cast<graphics::Renderer*>(graph->getNode("rendererMesh")) );
		auto viewMatrixUniform = context.create<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix")
			.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); });
		auto roughnessUniform = context.create<graphics::UniformVariableValue<float>>("uRoughness")
			.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); });
		pass->addBindable(program)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", glm::mat4(1.0f))
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			)
			.addBindable(viewMatrixUniform)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", kCubeMapProjectionMatrix)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			)
			.addBindable(roughnessUniform)
			.addBindable(source)
			.addBindable(
				context.create<graphics::UniformVariableValue<int>>("uCubeMap", 0)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			);
		auto technique = std::make_shared<graphics::Technique>();
		technique->addPass(pass);

		// Create the Cube RenderableMesh
		auto cubeRawMesh = MeshLoader::createBoxMesh("cube", glm::vec3(1.0f));
		auto cubeMesh = MeshLoader::createGraphicsMesh(context, cubeRawMesh);
		auto cubeRenderable = std::make_shared<graphics::RenderableMesh>(std::move(cubeMesh));
		cubeRenderable->addTechnique(technique);

		// Render to each of the mipmap levels
		for (int i = 0; i < kMaxMipPrefilterLevels; ++i) {
			// Render the Environment Map to each of the Faces of the CubeMap
			std::size_t currentSize = static_cast<std::size_t>(size * std::pow(0.5, i));

			frameBuffer.getParent()->execute([=](graphics::Context::Query& q) {
				q.getTBindable(frameBuffer)->attach(q.getTBindable(depthTexture), graphics::FrameBufferAttachment::kDepth);
			});

			float roughness = i / static_cast<float>(kMaxMipPrefilterLevels - 1);
			roughnessUniform.edit([=](graphics::UniformVariableValue<float>& uniform) {
				uniform.setValue(roughness);
			});

			for (int j = 0; j < 6; ++j) {
				frameBuffer.qedit([=](auto& q, auto& fb) {
					fb.attach(q.getTBindable(ret), graphics::FrameBufferAttachment::kColor0, i, 0, j);
				});
				viewMatrixUniform.edit([=](graphics::UniformVariableValue<glm::mat4>& uniform) {
					uniform.setValue(kCubeMapViewMatrices[j]);
				});

				context.execute([=](graphics::Context::Query& q) {
					int originX, originY;
					std::size_t dimensionsX, dimensionsY;
					graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
					graphics::GraphicsOperations::setViewport(0, 0, currentSize, currentSize);

					cubeRenderable->submit(q);
					graph->execute(q);

					graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);
				});
			}
		}

		return ret;
	}


	TextureRef TextureUtils::precomputeBRDF(graphics::Context& context, std::size_t size)
	{
		// Create the normal map
		auto ret = context.create<graphics::Texture>(graphics::TextureTarget::Texture2D);
		ret.edit([=](graphics::Texture& ret) {
			ret.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RG, graphics::ColorFormat::RG16f, size, size);
		});

		// Create the RenderGraph
		auto graph = std::make_shared<MyRenderGraph>(context);
		auto resources = graph->getNode("resources");
		auto frameBuffer = dynamic_cast<graphics::BindableRNodeOutput<graphics::FrameBuffer>*>(resources->findOutput("frameBuffer"))->getTBindable();

		auto depthTexture = context.create<graphics::Texture>(graphics::TextureTarget::Texture2D);
		depthTexture.edit([=](graphics::Texture& tex) {
			tex.setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, size, size);
		});
		frameBuffer.qedit([=](auto& q, auto& fb) {
			fb.attach(q.getTBindable(depthTexture), graphics::FrameBufferAttachment::kDepth);
		});

		// Create the Technique
		ProgramRef program;
		auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentPrecomputeBRDF.glsl", context, program);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return {};
		}

		auto pass = std::make_shared<graphics::Pass>( *dynamic_cast<graphics::Renderer*>(graph->getNode("rendererMesh")) );
		pass->addBindable(program)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", glm::mat4(1.0f))
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", glm::mat4(1.0f))
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", glm::mat4(1.0f))
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			);
		auto technique = std::make_shared<graphics::Technique>();
		technique->addPass(pass);

		// Create the plane
		RawMesh planeRawMesh;
		planeRawMesh.positions = { {-1.0f,-1.0f, 0.0f }, { 1.0f,-1.0f, 0.0f }, {-1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
		planeRawMesh.indices = { 0, 1, 2, 1, 3, 2, };
		auto planeMesh = MeshLoader::createGraphicsMesh(context, planeRawMesh);
		auto planeRenderable = std::make_shared<graphics::RenderableMesh>(std::move(planeMesh));
		planeRenderable->addTechnique(technique);

		// Render the the height map to the normal map
		frameBuffer.qedit([=](auto& q, auto& fb) {
			fb.attach(q.getTBindable(ret), graphics::FrameBufferAttachment::kColor0);
		});

		context.execute([=](graphics::Context::Query& q) {
			int originX, originY;
			std::size_t dimensionsX, dimensionsY;
			graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
			graphics::GraphicsOperations::setViewport(0, 0, size, size);

			planeRenderable->submit(q);
			graph->execute(q);

			graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);
		});

		return ret;
	}


	TextureRef TextureUtils::heightmapToNormalMapLocal(const TextureRef& source, std::size_t width, std::size_t height)
	{
		graphics::Context& context = *source.getParent();

		// Create the normal map
		auto ret = context.create<graphics::Texture>(graphics::TextureTarget::Texture2D);
		ret.edit([=](graphics::Texture& tex) {
			tex.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setImage(nullptr, graphics::TypeId::UnsignedByte, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB, width, height);
		});

		// Create the RenderGraph
		auto graph = std::make_shared<MyRenderGraph>(context);
		auto resources = graph->getNode("resources");
		auto frameBuffer = dynamic_cast<graphics::BindableRNodeOutput<graphics::FrameBuffer>*>(resources->findOutput("frameBuffer"))->getTBindable();

		auto depthTexture = context.create<graphics::Texture>(graphics::TextureTarget::Texture2D);
		depthTexture.edit([=](graphics::Texture& tex) {
			tex.setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, width, height);
		});
		frameBuffer.qedit([=](auto& q, auto& fb) {
			fb.attach(q.getTBindable(depthTexture), graphics::FrameBufferAttachment::kDepth);
			fb.attach(q.getTBindable(ret), graphics::FrameBufferAttachment::kColor0);
		});

		// Create the Technique
		ProgramRef program;
		auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentToNormalLocal.glsl", context, program);
		if (!result) {
			SOMBRA_ERROR_LOG << result.description();
			return {};
		}

		source.edit([](graphics::Texture& tex) { tex.setTextureUnit(0); });
		auto pass = std::make_shared<graphics::Pass>( *dynamic_cast<graphics::Renderer*>(graph->getNode("rendererMesh")) );
		pass->addBindable(program)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", glm::mat4(1.0f))
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", glm::mat4(1.0f))
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", glm::mat4(1.0f))
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			)
			.addBindable(
				context.create<graphics::UniformVariableValue<glm::vec2>>("uResolution", glm::vec2(width, height))
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			)
			.addBindable(source)
			.addBindable(
				context.create<graphics::UniformVariableValue<int>>("uHeightMap", 0)
					.qedit([=](auto& q, auto& uniform) { uniform.load(*q.getTBindable(program)); })
			);
		auto technique = std::make_shared<graphics::Technique>();
		technique->addPass(pass);

		// Create the plane
		RawMesh planeRawMesh;
		planeRawMesh.positions = { {-1.0f,-1.0f, 0.0f }, { 1.0f,-1.0f, 0.0f }, {-1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
		planeRawMesh.indices = { 0, 1, 2, 1, 3, 2, };
		auto planeMesh = MeshLoader::createGraphicsMesh(context, planeRawMesh);
		auto planeRenderable = std::make_shared<graphics::RenderableMesh>(std::move(planeMesh));
		planeRenderable->addTechnique(technique);

		// Render the the height map to the normal map
		context.execute([=](graphics::Context::Query& q) {
			int originX, originY;
			std::size_t dimensionsX, dimensionsY;
			graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
			graphics::GraphicsOperations::setViewport(0, 0, width, height);

			planeRenderable->submit(q);
			graph->execute(q);

			graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);
		});

		return ret;
	}


	template <typename T>
	Image<T> TextureUtils::textureToImage(
		const TextureRef& source,
		graphics::TypeId type, graphics::ColorFormat color,
		std::size_t width, std::size_t height
	) {
		Image<T> ret;
		switch (color) {
			case graphics::ColorFormat::R:		ret.channels = 1;	break;
			case graphics::ColorFormat::RG:		ret.channels = 2;	break;
			case graphics::ColorFormat::RGB:	ret.channels = 3;	break;
			case graphics::ColorFormat::RGBA:	ret.channels = 4;	break;
			default:							ret.channels = 1;	break;
		}

		ret.width = width;
		ret.height = height;
		ret.pixels = std::unique_ptr<T[]>(new T[ret.width * ret.height * ret.channels]);

		source.edit([&](graphics::Texture& tex) {
			tex.getImage(type, color, ret.pixels.get());
		});
		source.getParent()->wait();

		return ret;
	}


	template Image<unsigned char> TextureUtils::textureToImage<unsigned char>(
		const TextureRef&, graphics::TypeId, graphics::ColorFormat, std::size_t, std::size_t
	);
	template Image<float> TextureUtils::textureToImage<float>(
		const TextureRef&, graphics::TypeId, graphics::ColorFormat, std::size_t, std::size_t
	);

}
