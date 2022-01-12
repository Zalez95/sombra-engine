#ifndef DEFAULT_SCENE_H
#define DEFAULT_SCENE_H

#include <se/graphics/Renderer.h>
#include <se/graphics/GraphicsEngine.h>
#include <se/graphics/core/UniformVariable.h>
#include <se/graphics/core/GraphicsOperations.h>
#include <se/app/io/MeshLoader.h>
#include <se/app/io/ShaderLoader.h>
#include <se/app/graphics/RenderableShader.h>
#include <se/app/graphics/TextureUtils.h>
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
		auto& context = scene.application.getExternalTools().graphicsEngine->getContext();
		auto& renderGraph = scene.application.getExternalTools().graphicsEngine->getRenderGraph();

		// Default Scene resources
		auto cubeRawMesh = MeshLoader::createBoxMesh("cube", glm::vec3(1.0f));
		cubeRawMesh.normals = MeshLoader::calculateNormals(cubeRawMesh.positions, cubeRawMesh.indices);
		cubeRawMesh.tangents = MeshLoader::calculateTangents(cubeRawMesh.positions, cubeRawMesh.texCoords, cubeRawMesh.indices);
		auto cubeMesh = MeshLoader::createGraphicsMesh(context, cubeRawMesh);
		auto cubeMeshResource = scene.repository.insert(std::make_shared<MeshRef>(cubeMesh), "cube");
		cubeMeshResource.setFakeUser();

		auto pointLightSPtr = std::make_shared<LightSource>(scene.application.getEventManager(), LightSource::Type::Point);
		pointLightSPtr->setIntensity(10.0f);
		pointLightSPtr->setRange(20.0f);
		auto pointLightResource = scene.repository.insert<LightSource>(std::move(pointLightSPtr), "pointLight");
		pointLightResource.setFakeUser();

		static constexpr float kChessPixels[] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
		auto chessTexture = context.create<Texture>(TextureTarget::Texture2D);
		chessTexture.edit([](Texture& tex) {
			tex.setImage(kChessPixels, TypeId::Float, ColorFormat::RGB, ColorFormat::RGB, 2, 2)
				.setFiltering(TextureFilter::Nearest, TextureFilter::Nearest)
				.setWrapping(TextureWrap::Repeat, TextureWrap::Repeat);
		});
		auto chessTextureResource = scene.repository.insert(std::make_shared<TextureRef>(chessTexture), "chessTexture");
		chessTextureResource.setFakeUser();

		auto normalMapChessTexture = TextureUtils::heightmapToNormalMapLocal(*chessTextureResource, 2, 2);
		auto normalMapChessTextureResource = scene.repository.insert(std::make_shared<TextureRef>(normalMapChessTexture), "normalMapChessTexture");
		normalMapChessTextureResource.setFakeUser();

		chessTextureResource->edit([](Texture& tex) { tex.setTextureUnit(SplatmapMaterial::TextureUnits::kHeightMap); });
		normalMapChessTextureResource->edit([](Texture& tex) { tex.setTextureUnit(SplatmapMaterial::TextureUnits::kNormalMap); });

		ProgramRef programShadow;
		auto result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, nullptr, context, programShadow);
		if (!result) {
			throw std::runtime_error("Couldn't create programShadow: " + std::string(result.description()));
		}
		auto programShadowResource = scene.repository.insert(std::make_shared<ProgramRef>(programShadow), "programShadow");
		programShadowResource.setFakeUser();
		programShadowResource.getResource().setPath("res/shaders/vertex3D.glsl||");

		ProgramRef programShadowSkinning;
		result = ShaderLoader::createProgram("res/shaders/vertex3DSkinning.glsl", nullptr, nullptr, context, programShadowSkinning);
		if (!result) {
			throw std::runtime_error("Couldn't create programShadowSkinning: " + std::string(result.description()));
		}
		auto programShadowSkinningResource = scene.repository.insert(std::make_shared<ProgramRef>(programShadowSkinning), "programShadowSkinning");
		programShadowSkinningResource.setFakeUser();
		programShadowSkinningResource.getResource().setPath("res/shaders/vertex3DSkinning.glsl||");

		ProgramRef programShadowTerrain;
		result = ShaderLoader::createProgram("res/shaders/vertexTerrain.glsl", "res/shaders/geometryTerrain.glsl", nullptr, context, programShadowTerrain);
		if (!result) {
			throw std::runtime_error("Couldn't create programShadowTerrain: " + std::string(result.description()));
		}
		auto programShadowTerrainResource = scene.repository.insert(std::make_shared<ProgramRef>(programShadowTerrain), "programShadowTerrain");
		programShadowTerrainResource.setFakeUser();
		programShadowTerrainResource.getResource().setPath("res/shaders/vertexTerrain.glsl|res/shaders/geometryTerrain.glsl|");

		ProgramRef programSky;
		result = ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentSkyBox.glsl", context, programSky);
		if (!result) {
			throw std::runtime_error("Couldn't create programSky: " + std::string(result.description()));
		}
		auto programSkyResource = scene.repository.insert(std::make_shared<ProgramRef>(programSky), "programSky");
		programSkyResource.setFakeUser();
		programSkyResource.getResource().setPath("res/shaders/vertex3D.glsl||res/shaders/fragmentSkyBox.glsl");

		ProgramRef programGBufMaterial;
		result = ShaderLoader::createProgram("res/shaders/vertexNormalMap.glsl", nullptr, "res/shaders/fragmentGBufMaterial.glsl", context, programGBufMaterial);
		if (!result) {
			throw std::runtime_error("Couldn't create programGBufMaterial: " + std::string(result.description()));
		}
		auto programGBufMaterialResource = scene.repository.insert(std::make_shared<ProgramRef>(programGBufMaterial), "programGBufMaterial");
		programGBufMaterialResource.setFakeUser();
		programGBufMaterialResource.getResource().setPath("res/shaders/vertexNormalMap.glsl||res/shaders/fragmentGBufMaterial.glsl");

		ProgramRef programGBufMaterialSkinning;
		result = ShaderLoader::createProgram("res/shaders/vertexNormalMapSkinning.glsl", nullptr, "res/shaders/fragmentGBufMaterial.glsl", context, programGBufMaterialSkinning);
		if (!result) {
			throw std::runtime_error("Couldn't create programGBufMaterialSkinning: " + std::string(result.description()));
		}
		auto programGBufMaterialSkinningResource = scene.repository.insert(std::make_shared<ProgramRef>(programGBufMaterialSkinning), "programGBufMaterialSkinning");
		programGBufMaterialSkinningResource.setFakeUser();
		programGBufMaterialSkinningResource.getResource().setPath("res/shaders/vertexNormalMapSkinning.glsl||res/shaders/fragmentGBufMaterial.glsl");

		ProgramRef programGBufSplatmap;
		result = ShaderLoader::createProgram("res/shaders/vertexTerrain.glsl", "res/shaders/geometryTerrain.glsl", "res/shaders/fragmentGBufSplatmap.glsl", context, programGBufSplatmap);
		if (!result) {
			throw std::runtime_error("Couldn't create programGBufSplatmap: " + std::string(result.description()));
		}
		auto programGBufSplatmapResource = scene.repository.insert(std::make_shared<ProgramRef>(programGBufSplatmap), "programGBufSplatmap");
		programGBufSplatmapResource.setFakeUser();
		programGBufSplatmapResource.getResource().setPath("res/shaders/vertexTerrain.glsl|res/shaders/geometryTerrain.glsl|res/shaders/fragmentGBufSplatmap.glsl");

		ProgramRef programGBufParticles;
		result = ShaderLoader::createProgram("res/shaders/vertexParticlesFaceCamera.glsl", nullptr, "res/shaders/fragmentGBufMaterial.glsl", context, programGBufParticles);
		if (!result) {
			throw std::runtime_error("Couldn't create programGBufParticles: " + std::string(result.description()));
		}
		auto programGBufParticlesResource = scene.repository.insert(std::make_shared<ProgramRef>(programGBufParticles), "programGBufParticles");
		programGBufParticlesResource.setFakeUser();
		programGBufParticlesResource.getResource().setPath("res/shaders/vertexParticlesFaceCamera.glsl||res/shaders/fragmentGBufMaterial.glsl");

		auto shadowMeshProxyRenderer = static_cast<Renderer*>(renderGraph.getNode("shadowMeshProxyRenderer"));
		auto shadowTerrainProxyRenderer = static_cast<Renderer*>(renderGraph.getNode("shadowTerrainProxyRenderer"));
		auto gBufferRendererTerrain = static_cast<Renderer*>(renderGraph.getNode("gBufferRendererTerrain"));
		auto gBufferRendererMesh = static_cast<Renderer*>(renderGraph.getNode("gBufferRendererMesh"));
		auto gBufferRendererParticles = static_cast<Renderer*>(renderGraph.getNode("gBufferRendererParticles"));

		auto stepShadowResource = scene.repository.insert(std::make_shared<RenderableShaderStep>(*shadowMeshProxyRenderer), "stepShadow");
		stepShadowResource.setFakeUser();
		stepShadowResource->addResource(programShadowResource);

		auto stepShadowSkinningResource = scene.repository.insert(std::make_shared<RenderableShaderStep>(*shadowMeshProxyRenderer), "stepShadowSkinning");
		stepShadowSkinningResource.setFakeUser();
		stepShadowSkinningResource->addResource(programShadowSkinningResource);

		auto stepShadowTerrainResource = scene.repository.insert(std::make_shared<RenderableShaderStep>(*shadowTerrainProxyRenderer), "stepShadowTerrain");
		stepShadowTerrainResource.setFakeUser();
		ShaderLoader::addHeightMapBindables(stepShadowTerrainResource, chessTextureResource, normalMapChessTextureResource, 2.0f, 0.5f, programShadowTerrainResource);

		auto stepDefaultResource = scene.repository.insert(std::make_shared<RenderableShaderStep>(*gBufferRendererMesh), "stepDefault");
		stepDefaultResource.setFakeUser();
		ShaderLoader::addMaterialBindables(
			stepDefaultResource,
			Material{
				PBRMetallicRoughness{ glm::vec4(1.0f, 0.0f, 0.862f, 1.0f), chessTextureResource, 0.2f, 0.5f, {} },
				{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), AlphaMode::Opaque, 0.5f, false
			},
			programGBufMaterialResource
		);

		SplatmapMaterial sMaterial;
		sMaterial.splatmapTexture = chessTextureResource;
		sMaterial.materials.push_back(BasicMaterial{
			PBRMetallicRoughness{ glm::vec4(1.0f, 0.0f, 0.862f, 1.0f), {}, 0.2f, 0.5f, {} },
			{}, 1.0f
		});
		auto stepDefaultTerrainResource = scene.repository.insert(std::make_shared<RenderableShaderStep>(*gBufferRendererTerrain), "stepDefaultTerrain");
		stepDefaultTerrainResource.setFakeUser();
		ShaderLoader::addSplatmapMaterialBindables(stepDefaultTerrainResource, sMaterial, programGBufSplatmapResource);
		ShaderLoader::addHeightMapBindables(stepDefaultTerrainResource, chessTextureResource, normalMapChessTextureResource, 2.0f, 0.5f, programGBufSplatmapResource);

		auto stepDefaultParticlesResource = scene.repository.insert(std::make_shared<RenderableShaderStep>(*gBufferRendererParticles), "stepDefaultParticles");
		stepDefaultParticlesResource.setFakeUser();
		ShaderLoader::addMaterialBindables(
			stepDefaultParticlesResource,
			Material{
				PBRMetallicRoughness{ glm::vec4(1.0f, 0.0f, 0.862f, 1.0f), chessTextureResource, 0.2f, 0.5f, {} },
				{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), AlphaMode::Opaque, 0.5f, false
			},
			programGBufParticlesResource
		);

		auto shaderDefaultResource = scene.repository.insert(std::make_shared<RenderableShader>(scene.application.getEventManager()), "shaderDefault");
		shaderDefaultResource.setFakeUser();
		shaderDefaultResource->addStep(stepShadowResource)
			.addStep(stepDefaultResource);

		auto shaderDefaultTerrainResource = scene.repository.insert(std::make_shared<RenderableShader>(scene.application.getEventManager()), "shaderDefaultTerrain");
		shaderDefaultTerrainResource.setFakeUser();
		shaderDefaultTerrainResource->addStep(stepShadowTerrainResource)
			.addStep(stepDefaultTerrainResource);

		auto shaderDefaultParticlesResource = scene.repository.insert(std::make_shared<RenderableShader>(scene.application.getEventManager()), "shaderDefaultParticles");
		shaderDefaultParticlesResource.setFakeUser();
		shaderDefaultParticlesResource->addStep(stepDefaultParticlesResource);

		// Default Scene entities
		scene.application.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
			auto cubeEntity = query.addEntity();
			scene.entities.push_back(cubeEntity);
			query.emplaceComponent<TagComponent>(cubeEntity, true, "cube");
			auto cubeTransforms = query.emplaceComponent<TransformsComponent>(cubeEntity);
			cubeTransforms->position = { 0.0f, 0.5f, 0.0f };
			auto cubeMeshComponent = query.emplaceComponent<MeshComponent>(cubeEntity);
			std::size_t iRMesh = cubeMeshComponent->add(false, cubeMeshResource);
			cubeMeshComponent->addRenderableShader(iRMesh, shaderDefaultResource);
		});

		scene.application.getEntityDatabase().executeQuery([&](se::app::EntityDatabase::Query& query) {
			auto lightEntity = query.addEntity();
			scene.entities.push_back(lightEntity);
			query.emplaceComponent<TagComponent>(lightEntity, true, "pointLight");
			auto lightComponent = query.emplaceComponent<LightComponent>(lightEntity);
			lightComponent->setSource(pointLightResource);
			auto lightTransforms = query.emplaceComponent<TransformsComponent>(lightEntity);
			lightTransforms->position = { 3.0f, 7.5f, -1.0f };
		});
	}

}

#endif		// DEFAULT_SCENE_H
