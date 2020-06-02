#include <cassert>
#include "se/app/loaders/TerrainLoader.h"
#include "se/app/loaders/TechniqueLoader.h"
#include "se/app/graphics/Material.h"
#include "se/graphics/3D/RenderableTerrain.h"
#include "se/physics/RigidBody.h"
#include "se/collision/TerrainCollider.h"

namespace se::app {

	TerrainLoader::EntityUPtr TerrainLoader::createTerrain(
		const std::string& name, float size, float maxHeight,
		const Image& heightMap, const std::vector<float>& lodDistances,
		const SplatmapMaterial& terrainMaterial, const char* programName
	) {
		glm::vec3 scaleVector(size, 2.0f * maxHeight, size);

		// Entity
		auto entity = std::make_unique<app::Entity>(name);
		entity->scale = scaleVector;

		// Graphics data
		auto renderableTerrain = createTerrainRenderable(size, maxHeight, heightMap, lodDistances, terrainMaterial, programName);
		mGraphicsManager.addTerrainEntity(entity.get(), std::move(renderableTerrain));

		// Physics data
		physics::RigidBodyConfig config(0.2f);
		config.frictionCoefficient = 1.0f;
		auto rb = std::make_unique<physics::RigidBody>(config, physics::RigidBodyData());
		mPhysicsManager.addEntity(entity.get(), std::move(rb));

		// Collider data
		auto terrainCollider = createTerrainCollider(heightMap, scaleVector);
		mCollisionManager.addEntity(entity.get(), std::move(terrainCollider));

		return entity;
	}

// Private functions
	TerrainLoader::RenderableTerrainUPtr TerrainLoader::createTerrainRenderable(
		float size, float maxHeight,
		const Image& heightMap, const std::vector<float>& lodDistances,
		const SplatmapMaterial& terrainMaterial, const char* programName
	) {
		auto heightMapTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
		heightMapTexture->setTextureUnit(SplatmapMaterial::TextureUnits::kHeightMap)
			.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear)
			.setWrapping(se::graphics::TextureWrap::ClampToEdge, se::graphics::TextureWrap::ClampToEdge)
			.setImage(
				heightMap.pixels.get(), graphics::TypeId::UnsignedByte, graphics::ColorFormat::Red, graphics::ColorFormat::Red,
				heightMap.width, heightMap.height
			);

		auto program = mGraphicsManager.getProgramRepository().find(programName);
		auto terrainPass = mGraphicsManager.createPass3D(program, true);
		TechniqueLoader::addSplatmapMaterialBindables(terrainPass, terrainMaterial, program);
		auto terrainTechnique = std::make_shared<se::graphics::Technique>();
		terrainTechnique->addPass(terrainPass);

		auto renderable = std::make_unique<graphics::RenderableTerrain>(size, lodDistances);
		renderable->addBindable(std::move(heightMapTexture))
			.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
				"uHeightMap", *program, SplatmapMaterial::TextureUnits::kHeightMap
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uXZSize", *program, size
			))
			.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
				"uMaxHeight", *program, maxHeight
			));

		renderable->addTechnique(terrainTechnique);

		return renderable;
	}


	TerrainLoader::TerrainColliderUPtr TerrainLoader::createTerrainCollider(const Image& heightMap, const glm::vec3& scaleVector)
	{
		const std::size_t xSize = heightMap.width;
		const std::size_t zSize = heightMap.height;

		std::vector<float> heights;
		heights.reserve(xSize * zSize);
		for (std::size_t z = 0; z < zSize; ++z) {
			for (std::size_t x = 0; x < xSize; ++x) {
				heights.push_back( getHeight(heightMap, x, z) );
			}
		}

		auto collider = std::make_unique<collision::TerrainCollider>(heights, xSize, zSize);
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), scaleVector);
		collider->setTransforms(scale);

		return collider;
	}


	float TerrainLoader::getHeight(const Image& heightMap, std::size_t x, std::size_t z)
	{
		assert(x < heightMap.width && "x must be smaller than the image width");
		assert(z < heightMap.height && "z must be smaller than the image height");

		std::byte* heightMapPixels = heightMap.pixels.get();
		std::byte h = heightMapPixels[z * heightMap.width + x];

		return std::to_integer<unsigned char>(h) / static_cast<float>(kMaxColor) - 0.5f;
	}

}
