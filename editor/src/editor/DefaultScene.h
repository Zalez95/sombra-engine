#ifndef DEFAULT_SCENE_H
#define DEFAULT_SCENE_H

#include <se/graphics/Renderer.h>
#include <se/graphics/GraphicsEngine.h>
#include <se/graphics/core/UniformVariable.h>
#include <se/graphics/core/GraphicsOperations.h>
#include <se/app/io/MeshLoader.h>
#include <se/app/io/ShaderLoader.h>
#include <se/app/graphics/RenderableShader.h>
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
		auto cubeMeshSPtr = std::make_shared<Mesh>(MeshLoader::createGraphicsMesh(cubeRawMesh));
		auto cubeMesh = scene.repository.insert<Mesh>(std::move(cubeMeshSPtr), "cube");
		cubeMesh.setFakeUser();

		auto pointLightSPtr = std::make_shared<LightSource>(LightSource::Type::Point);
		pointLightSPtr->range = 20.0f;
		pointLightSPtr->intensity = 10.0f;
		auto pointLight = scene.repository.insert<LightSource>(std::move(pointLightSPtr), "pointLight");
		pointLight.setFakeUser();

		float pixels[] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
		auto chessTextureSPtr = std::make_shared<Texture>(TextureTarget::Texture2D);
		chessTextureSPtr->setImage(pixels, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB, 2, 2)
			.setFiltering(TextureFilter::Nearest, TextureFilter::Nearest)
			.setWrapping(TextureWrap::Repeat, TextureWrap::Repeat);
		auto chessTexture = scene.repository.insert<Texture>(std::move(chessTextureSPtr), "chessTexture");
		chessTexture.setFakeUser();

		std::shared_ptr<Program> programShadowSPtr;
		auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, nullptr, programShadowSPtr);
		if (!result) {
			throw std::runtime_error("Couldn't create programShadow: " + std::string(result.description()));
		}
		auto programShadow = scene.repository.insert<Program>(std::move(programShadowSPtr), "programShadow");
		programShadow.setFakeUser();
		programShadow.getResource().setPath("res/shaders/vertex3D.glsl||");

		std::shared_ptr<Program> programShadowSkinningSPtr;
		result = ShaderLoader::createProgram("res/shaders/vertex3DSkinning.glsl", nullptr, nullptr, programShadowSkinningSPtr);
		if (!result) {
			throw std::runtime_error("Couldn't create programShadowSkinning: " + std::string(result.description()));
		}
		auto programShadowSkinning = scene.repository.insert<Program>(std::move(programShadowSkinningSPtr), "programShadowSkinning");
		programShadowSkinning.setFakeUser();
		programShadowSkinning.getResource().setPath("res/shaders/vertex3DSkinning.glsl||");

		std::shared_ptr<Program> programShadowTerrainSPtr;
		result = ShaderLoader::createProgram("res/shaders/vertexTerrain.glsl", "res/shaders/geometryTerrain.glsl", nullptr, programShadowTerrainSPtr);
		if (!result) {
			throw std::runtime_error("Couldn't create programShadowTerrain: " + std::string(result.description()));
		}
		auto programShadowTerrain = scene.repository.insert<Program>(std::move(programShadowTerrainSPtr), "programShadowTerrain");
		programShadowTerrain.setFakeUser();
		programShadowTerrain.getResource().setPath("res/shaders/vertexTerrain.glsl|res/shaders/geometryTerrain.glsl|");

		std::shared_ptr<Program> programSkySPtr;
		result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentSkyBox.glsl", programSkySPtr);
		if (!result) {
			throw std::runtime_error("Couldn't create programSky: " + std::string(result.description()));
		}
		auto programSky = scene.repository.insert<Program>(std::move(programSkySPtr), "programSky");
		programSky.setFakeUser();
		programSky.getResource().setPath("res/shaders/vertex3D.glsl||res/shaders/fragmentSkyBox.glsl");

		std::shared_ptr<Program> programGBufMaterialSPtr;
		result = ShaderLoader::createProgram("res/shaders/vertexNormalMap.glsl", nullptr, "res/shaders/fragmentGBufMaterial.glsl", programGBufMaterialSPtr);
		if (!result) {
			throw std::runtime_error("Couldn't create programGBufMaterial: " + std::string(result.description()));
		}
		auto programGBufMaterial = scene.repository.insert<Program>(std::move(programGBufMaterialSPtr), "programGBufMaterial");
		programGBufMaterial.setFakeUser();
		programGBufMaterial.getResource().setPath("res/shaders/vertexNormalMap.glsl||res/shaders/fragmentGBufMaterial.glsl");

		std::shared_ptr<Program> programGBufMaterialSkinningSPtr;
		result = ShaderLoader::createProgram("res/shaders/vertexNormalMapSkinning.glsl", nullptr, "res/shaders/fragmentGBufMaterial.glsl", programGBufMaterialSkinningSPtr);
		if (!result) {
			throw std::runtime_error("Couldn't create programGBufMaterialSkinning: " + std::string(result.description()));
		}
		auto programGBufMaterialSkinning = scene.repository.insert<Program>(std::move(programGBufMaterialSkinningSPtr), "programGBufMaterialSkinning");
		programGBufMaterialSkinning.setFakeUser();
		programGBufMaterialSkinning.getResource().setPath("res/shaders/vertexNormalMapSkinning.glsl||res/shaders/fragmentGBufMaterial.glsl");

		std::shared_ptr<Program> programGBufSplatmapSPtr;
		result = ShaderLoader::createProgram("res/shaders/vertexTerrain.glsl", "res/shaders/geometryTerrain.glsl", "res/shaders/fragmentGBufSplatmap.glsl", programGBufSplatmapSPtr);
		if (!result) {
			throw std::runtime_error("Couldn't create programGBufSplatmap: " + std::string(result.description()));
		}
		auto programGBufSplatmap = scene.repository.insert<Program>(std::move(programGBufSplatmapSPtr), "programGBufSplatmap");
		programGBufSplatmap.setFakeUser();
		programGBufSplatmap.getResource().setPath("res/shaders/vertexTerrain.glsl|res/shaders/geometryTerrain.glsl|res/shaders/fragmentGBufSplatmap.glsl");

		std::shared_ptr<Program> programGBufParticlesSPtr;
		result = ShaderLoader::createProgram("res/shaders/vertexParticlesFaceCamera.glsl", nullptr, "res/shaders/fragmentGBufMaterial.glsl", programGBufParticlesSPtr);
		if (!result) {
			throw std::runtime_error("Couldn't create programGBufParticles: " + std::string(result.description()));
		}
		auto programGBufParticles = scene.repository.insert<Program>(std::move(programGBufParticlesSPtr), "programGBufParticles");
		programGBufParticles.setFakeUser();
		programGBufParticles.getResource().setPath("res/shaders/vertexParticlesFaceCamera.glsl||res/shaders/fragmentGBufMaterial.glsl");

		auto shadowRendererMesh = static_cast<Renderer*>(scene.application.getExternalTools().graphicsEngine->getRenderGraph().getNode("shadowRendererMesh"));
		auto gBufferRendererMesh = static_cast<Renderer*>(scene.application.getExternalTools().graphicsEngine->getRenderGraph().getNode("gBufferRendererMesh"));
		auto gBufferRendererParticles = static_cast<Renderer*>(scene.application.getExternalTools().graphicsEngine->getRenderGraph().getNode("gBufferRendererParticles"));

		auto stepShadow = scene.repository.insert(std::make_shared<RenderableShaderStep>(*shadowRendererMesh), "stepShadow");
		stepShadow.setFakeUser();
		stepShadow->addResource(programShadow);

		auto stepShadowSkinning = scene.repository.insert(std::make_shared<RenderableShaderStep>(*shadowRendererMesh), "stepShadowSkinning");
		stepShadowSkinning.setFakeUser();
		stepShadowSkinning->addResource(programShadowSkinning);

		auto stepShadowTerrain = scene.repository.insert(std::make_shared<RenderableShaderStep>(*shadowRendererMesh), "stepShadowTerrain");
		stepShadowTerrain.setFakeUser();
		ShaderLoader::addHeightMapBindables(stepShadowTerrain, chessTexture, 2.0f, 0.5f, programShadowTerrain);

		auto stepDefault = scene.repository.insert(std::make_shared<RenderableShaderStep>(*gBufferRendererMesh), "stepDefault");
		stepDefault.setFakeUser();
		ShaderLoader::addMaterialBindables(
			stepDefault,
			Material{
				PBRMetallicRoughness{ glm::vec4(1.0f, 0.0f, 0.862f, 1.0f), chessTexture, 0.2f, 0.5f, {} },
				{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), AlphaMode::Opaque, 0.5f, false
			},
			programGBufMaterial
		);

		SplatmapMaterial sMaterial;
		sMaterial.splatmapTexture = chessTexture;
		sMaterial.materials.push_back(BasicMaterial{
			PBRMetallicRoughness{ glm::vec4(1.0f, 0.0f, 0.862f, 1.0f), chessTexture, 0.2f, 0.5f, {} },
			{}, 1.0f
		});
		auto stepDefaultTerrain = scene.repository.insert(std::make_shared<RenderableShaderStep>(*gBufferRendererMesh), "stepDefaultTerrain");
		stepDefaultTerrain.setFakeUser();
		ShaderLoader::addSplatmapMaterialBindables(stepDefaultTerrain, sMaterial, programGBufSplatmap);
		ShaderLoader::addHeightMapBindables(stepDefaultTerrain, chessTexture, 2.0f, 0.5f, programGBufSplatmap);

		auto stepDefaultParticles = scene.repository.insert(std::make_shared<RenderableShaderStep>(*gBufferRendererParticles), "stepDefaultParticles");
		stepDefaultParticles.setFakeUser();
		ShaderLoader::addMaterialBindables(
			stepDefaultParticles,
			Material{
				PBRMetallicRoughness{ glm::vec4(1.0f, 0.0f, 0.862f, 1.0f), chessTexture, 0.2f, 0.5f, {} },
				{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), AlphaMode::Opaque, 0.5f, false
			},
			programGBufParticles
		);

		auto shaderDefault = scene.repository.insert(std::make_shared<RenderableShader>(scene.application.getEventManager()), "shaderDefault");
		shaderDefault.setFakeUser();
		shaderDefault->addStep(stepShadow)
			.addStep(stepDefault);

		auto shaderDefaultTerrain = scene.repository.insert(std::make_shared<RenderableShader>(scene.application.getEventManager()), "shaderDefaultTerrain");
		shaderDefaultTerrain.setFakeUser();
		shaderDefaultTerrain->addStep(stepShadowTerrain)
			.addStep(stepDefaultTerrain);

		auto shaderDefaultParticles = scene.repository.insert(std::make_shared<RenderableShader>(scene.application.getEventManager()), "shaderDefaultParticles");
		shaderDefaultParticles.setFakeUser();
		shaderDefaultParticles->addStep(stepDefaultParticles);

		// Default Scene entities
		auto cubeEntity = scene.application.getEntityDatabase().addEntity();
		scene.entities.push_back(cubeEntity);
		scene.application.getEntityDatabase().emplaceComponent<TagComponent>(cubeEntity, "cube");
		auto cubeTransforms = scene.application.getEntityDatabase().emplaceComponent<TransformsComponent>(cubeEntity);
		cubeTransforms->position = { 0.0f, 0.5f, 0.0f };
		auto cubeMeshComponent = scene.application.getEntityDatabase().emplaceComponent<MeshComponent>(cubeEntity);
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
