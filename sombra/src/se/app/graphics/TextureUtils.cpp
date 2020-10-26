#include <glm/gtc/matrix_transform.hpp>
#include "se/app/graphics/TextureUtils.h"
#include "se/app/loaders/MeshLoader.h"
#include "se/app/loaders/TechniqueLoader.h"
#include "se/graphics/RenderGraph.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/graphics/3D/RenderableMesh.h"
#include "se/graphics/FBClearNode.h"
#include "se/graphics/3D/Renderer3D.h"

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
		MyRenderGraph()
		{
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(getNode("resources"));
			auto frameBufferIndex = resources->addBindable(std::make_shared<graphics::FrameBuffer>());
			resources->addOutput(std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("frameBuffer", resources, frameBufferIndex));

			auto clearMask = graphics::FrameBufferMask::Mask().set(graphics::FrameBufferMask::kColor).set(graphics::FrameBufferMask::kDepth);
			auto fbClearNode = std::make_unique<graphics::FBClearNode>("fbClearNode", clearMask);

			auto renderer3D = std::make_unique<graphics::Renderer3D>("renderer3D");

			fbClearNode->findInput("input")->connect(resources->findOutput("frameBuffer"));
			renderer3D->findInput("target")->connect(fbClearNode->findOutput("output"));
			addNode(std::move(renderer3D));
			addNode(std::move(fbClearNode));

			prepareGraph();
		};
	};


	TextureUtils::TextureSPtr TextureUtils::equirectangularToCubeMap(TextureSPtr source, std::size_t size)
	{
		// Create the CubeMap
		auto ret = std::make_shared<graphics::Texture>(graphics::TextureTarget::CubeMap);
		for (int i = 0; i < 6; ++i) {
			ret->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB16f, size, size, 0, i);
		}
		ret->setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
			.setFiltering(graphics::TextureFilter::LinearMipMapLinear, graphics::TextureFilter::Linear);

		// Create the RenderGraph
		MyRenderGraph graph;
		auto frameBufferOutput = dynamic_cast<graphics::BindableRNodeOutput<graphics::FrameBuffer>*>(graph.getNode("resources")->findOutput("frameBuffer"));
		auto frameBuffer = std::dynamic_pointer_cast<graphics::FrameBuffer>(frameBufferOutput->getBindable());

		auto depthTexture = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2D);
		depthTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, size, size);
		frameBuffer->attach(*depthTexture, graphics::FrameBufferAttachment::kDepth);

		// Create the Technique
		std::shared_ptr<graphics::Program> program = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentEquiToCubeMap.glsl");
		auto viewMatrixUniform = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program);
		source->setTextureUnit(0);
		auto pass = std::make_shared<graphics::Pass>( *dynamic_cast<graphics::Renderer3D*>(graph.getNode("renderer3D")) );
		pass->addBindable(program)
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)))
			.addBindable(viewMatrixUniform)
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, kCubeMapProjectionMatrix))
			.addBindable(source)
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uEquirectangularMap", *program, 0));
		auto technique = std::make_shared<graphics::Technique>();
		technique->addPass(pass);

		// Create the Cube RenderableMesh
		auto cubeRawMesh = MeshLoader::createBoxMesh("cube", glm::vec3(1.0f));
		auto cubeMesh = std::make_unique<graphics::Mesh>(MeshLoader::createGraphicsMesh(cubeRawMesh));
		auto cubeRenderable = std::make_unique<graphics::RenderableMesh>(std::move(cubeMesh));
		cubeRenderable->addTechnique(technique);

		// Render the Environment Map to each of the Faces of the CubeMap
		int originX, originY;
		std::size_t dimensionsX, dimensionsY;
		graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
		graphics::GraphicsOperations::setViewport(0, 0, size, size);

		for (int i = 0; i < 6; ++i) {
			frameBuffer->attach(*ret, graphics::FrameBufferAttachment::kColor0, 0, 0, i);
			viewMatrixUniform->setValue(kCubeMapViewMatrices[i]);

			cubeRenderable->submit();
			graph.execute();
		}

		graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);

		ret->generateMipMap();
		return ret;
	}


	TextureUtils::TextureSPtr TextureUtils::convoluteCubeMap(TextureSPtr source, std::size_t size)
	{
		// Create the CubeMap
		auto ret = std::make_shared<graphics::Texture>(graphics::TextureTarget::CubeMap);
		for (int i = 0; i < 6; ++i) {
			ret->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB16f, size, size, 0, i);
		}
		ret->setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear)
			.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge);

		// Create the RenderGraph
		MyRenderGraph graph;
		auto frameBufferOutput = dynamic_cast<graphics::BindableRNodeOutput<graphics::FrameBuffer>*>(graph.getNode("resources")->findOutput("frameBuffer"));
		auto frameBuffer = std::dynamic_pointer_cast<graphics::FrameBuffer>(frameBufferOutput->getBindable());

		auto depthTexture = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2D);
		depthTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, size, size);
		frameBuffer->attach(*depthTexture, graphics::FrameBufferAttachment::kDepth);

		// Create the Technique
		std::shared_ptr<graphics::Program> program = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentConvoluteCubeMap.glsl");
		auto viewMatrixUniform = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program);
		source->setTextureUnit(0);
		auto pass = std::make_shared<graphics::Pass>( *dynamic_cast<graphics::Renderer3D*>(graph.getNode("renderer3D")) );
		pass->addBindable(program)
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)))
			.addBindable(viewMatrixUniform)
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, kCubeMapProjectionMatrix))
			.addBindable(source)
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uCubeMap", *program, 0));
		auto technique = std::make_shared<graphics::Technique>();
		technique->addPass(pass);

		// Create the Cube RenderableMesh
		auto cubeRawMesh = MeshLoader::createBoxMesh("cube", glm::vec3(1.0f));
		auto cubeMesh = std::make_unique<graphics::Mesh>(MeshLoader::createGraphicsMesh(cubeRawMesh));
		auto cubeRenderable = std::make_unique<graphics::RenderableMesh>(std::move(cubeMesh));
		cubeRenderable->addTechnique(technique);

		// Render the Environment Map to each of the Faces of the CubeMap
		int originX, originY;
		std::size_t dimensionsX, dimensionsY;
		graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
		graphics::GraphicsOperations::setViewport(0, 0, size, size);

		for (int i = 0; i < 6; ++i) {
			frameBuffer->attach(*ret, graphics::FrameBufferAttachment::kColor0, 0, 0, i);
			viewMatrixUniform->setValue(kCubeMapViewMatrices[i]);

			cubeRenderable->submit();
			graph.execute();
		}

		graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);

		return ret;
	}


	TextureUtils::TextureSPtr TextureUtils::prefilterCubeMap(TextureSPtr source, std::size_t size)
	{
		// Create the CubeMap
		auto ret = std::make_shared<graphics::Texture>(graphics::TextureTarget::CubeMap);
		for (int i = 0; i < 6; ++i) {
			ret->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB16f, size, size, 0, i);
		}
		ret->setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
			.setFiltering(graphics::TextureFilter::LinearMipMapLinear, graphics::TextureFilter::Linear)
			.generateMipMap();

		// Create the RenderGraph
		MyRenderGraph graph;
		auto frameBufferOutput = dynamic_cast<graphics::BindableRNodeOutput<graphics::FrameBuffer>*>(graph.getNode("resources")->findOutput("frameBuffer"));
		auto frameBuffer = std::dynamic_pointer_cast<graphics::FrameBuffer>(frameBufferOutput->getBindable());

		auto depthTexture = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2D);
		depthTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, size, size)
			.generateMipMap();

		// Create the Technique
		std::shared_ptr<graphics::Program> program = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentPrefilterCubeMap.glsl");
		auto viewMatrixUniform = std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program);
		auto roughnessUniform = std::make_shared<graphics::UniformVariableValue<float>>("uRoughness", *program);
		source->setTextureUnit(0);
		auto pass = std::make_shared<graphics::Pass>( *dynamic_cast<graphics::Renderer3D*>(graph.getNode("renderer3D")) );
		pass->addBindable(program)
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)))
			.addBindable(viewMatrixUniform)
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, kCubeMapProjectionMatrix))
			.addBindable(roughnessUniform)
			.addBindable(source)
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uCubeMap", *program, 0));
		auto technique = std::make_shared<graphics::Technique>();
		technique->addPass(pass);

		// Create the Cube RenderableMesh
		auto cubeRawMesh = MeshLoader::createBoxMesh("cube", glm::vec3(1.0f));
		auto cubeMesh = std::make_unique<graphics::Mesh>(MeshLoader::createGraphicsMesh(cubeRawMesh));
		auto cubeRenderable = std::make_unique<graphics::RenderableMesh>(std::move(cubeMesh));
		cubeRenderable->addTechnique(technique);

		int originX, originY;
		std::size_t dimensionsX, dimensionsY;
		graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);

		// Render to each of the mipmap levels
		for (int i = 0; i < kMaxMipPrefilterLevels; ++i) {
			// Render the Environment Map to each of the Faces of the CubeMap
			std::size_t currentSize = static_cast<std::size_t>(size * std::pow(0.5, i));
			graphics::GraphicsOperations::setViewport(0, 0, currentSize, currentSize);

			frameBuffer->attach(*depthTexture, graphics::FrameBufferAttachment::kDepth, 0, i);

			float roughness = i / static_cast<float>(kMaxMipPrefilterLevels - 1);
			roughnessUniform->setValue(roughness);

			for (int j = 0; j < 6; ++j) {
				frameBuffer->attach(*ret, graphics::FrameBufferAttachment::kColor0, i, 0, j);
				viewMatrixUniform->setValue(kCubeMapViewMatrices[j]);

				cubeRenderable->submit();
				graph.execute();
			}
		}

		graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);

		return ret;
	}


	TextureUtils::TextureSPtr TextureUtils::precomputeBRDF(std::size_t size)
	{
		// Create the normal map
		auto ret = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2D);
		ret->setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear)
			.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
			.setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RG, graphics::ColorFormat::RG16f, size, size);

		// Create the RenderGraph
		MyRenderGraph graph;
		auto frameBufferOutput = dynamic_cast<graphics::BindableRNodeOutput<graphics::FrameBuffer>*>(graph.getNode("resources")->findOutput("frameBuffer"));
		auto frameBuffer = std::dynamic_pointer_cast<graphics::FrameBuffer>(frameBufferOutput->getBindable());

		auto depthTexture = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2D);
		depthTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, size, size);
		frameBuffer->attach(*depthTexture, graphics::FrameBufferAttachment::kDepth);

		// Create the Technique
		std::shared_ptr<graphics::Program> program = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentPrecomputeBRDF.glsl");
		auto pass = std::make_shared<graphics::Pass>( *dynamic_cast<graphics::Renderer3D*>(graph.getNode("renderer3D")) );
		pass->addBindable(program)
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)))
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program, glm::mat4(1.0f)))
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, glm::mat4(1.0f)));
		auto technique = std::make_shared<graphics::Technique>();
		technique->addPass(pass);

		// Create the plane
		RawMesh planeRawMesh;
		planeRawMesh.positions = { {-1.0f,-1.0f, 0.0f }, { 1.0f,-1.0f, 0.0f }, {-1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
		planeRawMesh.faceIndices = { 0, 1, 2, 1, 3, 2, };
		auto planeMesh = std::make_unique<graphics::Mesh>(MeshLoader::createGraphicsMesh(planeRawMesh));
		auto planeRenderable = std::make_unique<graphics::RenderableMesh>(std::move(planeMesh));
		planeRenderable->addTechnique(technique);

		// Render the the height map to the normal map
		int originX, originY;
		std::size_t dimensionsX, dimensionsY;
		graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
		graphics::GraphicsOperations::setViewport(0, 0, size, size);

		frameBuffer->attach(*ret, graphics::FrameBufferAttachment::kColor0);
		planeRenderable->submit();
		graph.execute();

		graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);

		return ret;
	}


	TextureUtils::TextureSPtr TextureUtils::heightmapToNormalMapLocal(TextureSPtr source, std::size_t width, std::size_t height)
	{
		// Create the normal map
		auto ret = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2D);
		ret->setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear)
			.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
			.setImage(nullptr, graphics::TypeId::UnsignedByte, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB, width, height);

		// Create the RenderGraph
		MyRenderGraph graph;
		auto frameBufferOutput = dynamic_cast<graphics::BindableRNodeOutput<graphics::FrameBuffer>*>(graph.getNode("resources")->findOutput("frameBuffer"));
		auto frameBuffer = std::dynamic_pointer_cast<graphics::FrameBuffer>(frameBufferOutput->getBindable());

		auto depthTexture = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2D);
		depthTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, width, height);
		frameBuffer->attach(*depthTexture, graphics::FrameBufferAttachment::kDepth);

		// Create the Technique
		std::shared_ptr<graphics::Program> program = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentToNormalLocal.glsl");
		source->setTextureUnit(0);
		auto pass = std::make_shared<graphics::Pass>( *dynamic_cast<graphics::Renderer3D*>(graph.getNode("renderer3D")) );
		pass->addBindable(program)
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)))
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program, glm::mat4(1.0f)))
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, glm::mat4(1.0f)))
			.addBindable(std::make_shared<graphics::UniformVariableValue<glm::vec2>>("uResolution", *program, glm::vec2(width, height)))
			.addBindable(source)
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uHeightMap", *program, 0));
		auto technique = std::make_shared<graphics::Technique>();
		technique->addPass(pass);

		// Create the plane
		RawMesh planeRawMesh;
		planeRawMesh.positions = { {-1.0f,-1.0f, 0.0f }, { 1.0f,-1.0f, 0.0f }, {-1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
		planeRawMesh.faceIndices = { 0, 1, 2, 1, 3, 2, };
		auto planeMesh = std::make_unique<graphics::Mesh>(MeshLoader::createGraphicsMesh(planeRawMesh));
		auto planeRenderable = std::make_unique<graphics::RenderableMesh>(std::move(planeMesh));
		planeRenderable->addTechnique(technique);

		// Render the the height map to the normal map
		int originX, originY;
		std::size_t dimensionsX, dimensionsY;
		graphics::GraphicsOperations::getViewport(originX, originY, dimensionsX, dimensionsY);
		graphics::GraphicsOperations::setViewport(0, 0, width, height);

		frameBuffer->attach(*ret, graphics::FrameBufferAttachment::kColor0);
		planeRenderable->submit();
		graph.execute();

		graphics::GraphicsOperations::setViewport(originX, originY, dimensionsX, dimensionsY);

		return ret;
	}

}
