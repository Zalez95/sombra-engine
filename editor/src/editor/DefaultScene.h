#ifndef DEFAULT_SCENE_H
#define DEFAULT_SCENE_H

#include <se/graphics/Renderer.h>
#include <se/graphics/GraphicsEngine.h>
#include <se/graphics/core/UniformVariable.h>
#include <se/graphics/core/GraphicsOperations.h>
#include <se/app/io/MeshLoader.h>
#include <se/app/io/ShaderLoader.h>
#include <se/app/RenderableShader.h>
#include <se/app/TransformsComponent.h>
#include <se/app/TagComponent.h>
#include <se/app/MeshComponent.h>
#include <se/app/LightComponent.h>
#include <se/app/Scene.h>

namespace editor {

	static void buildDefaultScene(se::app::Scene& scene)
	{
		using namespace se::app;
		using namespace se::graphics;

		// Default Scene resources
		auto cubeRawMesh = MeshLoader::createBoxMesh("cube", glm::vec3(1.0f));
		cubeRawMesh.normals = MeshLoader::calculateNormals(cubeRawMesh.positions, cubeRawMesh.indices);
		cubeRawMesh.tangents = MeshLoader::calculateTangents(cubeRawMesh.positions, cubeRawMesh.texCoords, cubeRawMesh.indices);
		auto cubeMesh = std::make_shared<Mesh>(MeshLoader::createGraphicsMesh(cubeRawMesh));

		auto pointLight = std::make_shared<LightSource>(LightSource::Type::Point);
		pointLight->range = 20.0f;
		pointLight->intensity = 10.0f;

		float pixels[] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
		auto chessTexture = std::make_shared<Texture>(TextureTarget::Texture2D);
		chessTexture->setImage(pixels, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB, 2, 2)
			.setFiltering(TextureFilter::Nearest, TextureFilter::Nearest)
			.setWrapping(TextureWrap::Repeat, TextureWrap::Repeat);

		std::shared_ptr<Program> programShadow;
		auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, nullptr, programShadow);
		if (!result) {
			throw std::runtime_error("Couldn't create programShadow: " + std::string(result.description()));
		}

		std::shared_ptr<Program> programShadowSkinning;
		result = ShaderLoader::createProgram("res/shaders/vertex3DSkinning.glsl", nullptr, nullptr, programShadowSkinning);
		if (!result) {
			throw std::runtime_error("Couldn't create programShadowSkinning: " + std::string(result.description()));
		}

		std::shared_ptr<Program> programShadowTerrain;
		result = ShaderLoader::createProgram("res/shaders/vertexTerrain.glsl", "res/shaders/geometryTerrain.glsl", nullptr, programShadowTerrain);
		if (!result) {
			throw std::runtime_error("Couldn't create programShadowTerrain: " + std::string(result.description()));
		}

		std::shared_ptr<Program> programSky;
		result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentSkyBox.glsl", programSky);
		if (!result) {
			throw std::runtime_error("Couldn't create programSky: " + std::string(result.description()));
		}

		std::shared_ptr<Program> programGBufMaterial;
		result = ShaderLoader::createProgram("res/shaders/vertexNormalMap.glsl", nullptr, "res/shaders/fragmentGBufMaterial.glsl", programGBufMaterial);
		if (!result) {
			throw std::runtime_error("Couldn't create programGBufMaterial: " + std::string(result.description()));
		}

		std::shared_ptr<Program> programGBufMaterialSkinning;
		result = ShaderLoader::createProgram("res/shaders/vertexNormalMapSkinning.glsl", nullptr, "res/shaders/fragmentGBufMaterial.glsl", programGBufMaterialSkinning);
		if (!result) {
			throw std::runtime_error("Couldn't create programGBufMaterialSkinning: " + std::string(result.description()));
		}

		std::shared_ptr<Program> programGBufSplatmap;
		result = ShaderLoader::createProgram("res/shaders/vertexTerrain.glsl", "res/shaders/geometryTerrain.glsl", "res/shaders/fragmentGBufSplatmap.glsl", programGBufSplatmap);
		if (!result) {
			throw std::runtime_error("Couldn't create programGBufSplatmap: " + std::string(result.description()));
		}

		auto shadowRenderer = static_cast<Renderer*>(scene.application.getExternalTools().graphicsEngine->getRenderGraph().getNode("shadowRenderer"));
		auto gBufferRenderer = static_cast<Renderer*>(scene.application.getExternalTools().graphicsEngine->getRenderGraph().getNode("gBufferRenderer"));

		auto passShadow = std::make_shared<Pass>(*shadowRenderer);
		passShadow->addBindable(programShadow);

		auto passShadowSkinning = std::make_shared<Pass>(*shadowRenderer);
		passShadowSkinning->addBindable(programShadowSkinning);

		auto passDefault = std::make_shared<Pass>(*gBufferRenderer);
		passDefault->addBindable(programGBufMaterial);
		ShaderLoader::addMaterialBindables(
			passDefault,
			Material{
				PBRMetallicRoughness{ glm::vec4(1.0f, 0.0f, 0.862f, 1.0f), chessTexture, 0.2f, 0.5f, {} },
				{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), AlphaMode::Opaque, 0.5f, false
			},
			programGBufMaterial
		);

		auto shaderDefault = std::make_shared<RenderableShader>(scene.application.getEventManager());
		shaderDefault->addPass(passShadow)
			.addPass(passDefault);

		scene.repository.add<Scene::Key, Mesh>("cube", cubeMesh);
		scene.repository.add<Scene::Key, LightSource>("pointLight", pointLight);
		scene.repository.add<Scene::Key, Texture>("chessTexture", chessTexture);
		scene.repository.add(Scene::Key("passShadow"), passShadow);
		scene.repository.add(Scene::Key("passShadowSkinning"), passShadowSkinning);
		scene.repository.add(Scene::Key("passDefault"), passDefault);
		scene.repository.add(Scene::Key("shaderDefault"), shaderDefault);
		scene.repository.add<Scene::Key, Program>("programShadow", std::move(programShadow));
		scene.repository.emplace<Scene::Key, ResourcePath<Program>>("programShadow", "res/shaders/vertex3D.glsl,,");
		scene.repository.add<Scene::Key, Program>("programShadowSkinning", std::move(programShadowSkinning));
		scene.repository.emplace<Scene::Key, ResourcePath<Program>>("programShadowSkinning", "res/shaders/vertex3DSkinning.glsl,,");
		scene.repository.add<Scene::Key, Program>("programShadowTerrain", std::move(programShadowTerrain));
		scene.repository.emplace<Scene::Key, ResourcePath<Program>>("programShadowTerrain", "res/shaders/vertexTerrain.glsl,res/shaders/geometryTerrain.glsl,");
		scene.repository.add<Scene::Key, Program>("programSky", std::move(programSky));
		scene.repository.emplace<Scene::Key, ResourcePath<Program>>("programSky", "res/shaders/vertex3D.glsl,,res/shaders/fragmentSkyBox.glsl");
		scene.repository.add<Scene::Key, Program>("programGBufMaterial", std::move(programGBufMaterial));
		scene.repository.emplace<Scene::Key, ResourcePath<Program>>("programGBufMaterial", "res/shaders/vertexNormalMap.glsl,,res/shaders/fragmentGBufMaterial.glsl");
		scene.repository.add<Scene::Key, Program>("programGBufMaterialSkinning", std::move(programGBufMaterialSkinning));
		scene.repository.emplace<Scene::Key, ResourcePath<Program>>("programGBufMaterialSkinning", "res/shaders/vertexNormalMapSkinning.glsl,,res/shaders/fragmentGBufMaterial.glsl");
		scene.repository.add<Scene::Key, Program>("programGBufSplatmap", std::move(programGBufSplatmap));
		scene.repository.emplace<Scene::Key, ResourcePath<Program>>("programGBufSplatmap", "res/shaders/vertexTerrain.glsl,res/shaders/geometryTerrain.glsl,res/shaders/fragmentGBufSplatmap.glsl");

		// Default Scene entities
		auto cubeEntity = scene.application.getEntityDatabase().addEntity();
		scene.entities.push_back(cubeEntity);
		scene.application.getEntityDatabase().emplaceComponent<TagComponent>(cubeEntity, "cube");
		auto cubeTransforms = scene.application.getEntityDatabase().emplaceComponent<TransformsComponent>(cubeEntity);
		cubeTransforms->position = { 0.0f, 0.5f, 0.0f };
		auto cubeMeshComponent = scene.application.getEntityDatabase().emplaceComponent<MeshComponent>(cubeEntity, scene.application.getEventManager(), cubeEntity);
		std::size_t iRMesh = cubeMeshComponent->add(false, cubeMesh);
		cubeMeshComponent->addRenderableShader(iRMesh, shaderDefault);

		auto lightEntity = scene.application.getEntityDatabase().addEntity();
		scene.entities.push_back(lightEntity);
		scene.application.getEntityDatabase().emplaceComponent<TagComponent>(lightEntity, "pointLight");
		auto lightComponent = scene.application.getEntityDatabase().emplaceComponent<LightComponent>(lightEntity);
		lightComponent->source = pointLight;
		auto lightTransforms = scene.application.getEntityDatabase().emplaceComponent<TransformsComponent>(lightEntity);
		lightTransforms->position = { 3.0f, 7.5f, -1.0f };
	}

}

#endif		// DEFAULT_SCENE_H
