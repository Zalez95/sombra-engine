#include <cassert>
#include "se/app/loaders/TerrainLoader.h"
#include "se/app/Entity.h"
#include "se/app/Image.h"
#include "se/graphics/3D/Material.h"
#include "se/graphics/3D/RenderableTerrain.h"
#include "se/physics/RigidBody.h"
#include "se/collision/TerrainCollider.h"

namespace se::app {

	TerrainLoader::EntityUPtr TerrainLoader::createTerrain(
		const std::string& name, float size, float maxHeight,
		const Image& heightMap, const std::vector<float>& lodDistances
	) {
		glm::vec3 scaleVector(size, 2.0f * maxHeight, size);

		// Entity
		auto entity = std::make_unique<app::Entity>(name);
		entity->scale = scaleVector;

		// Graphics data
		auto renderableTerrain = createTerrainRenderable(size, maxHeight, heightMap, lodDistances);
		mGraphicsManager.addTerrainEntity(entity.get(), std::move(renderableTerrain));

		// Physics data
		auto rb = std::make_unique<physics::RigidBody>(physics::RigidBodyConfig(0.2f), physics::RigidBodyData());
		mPhysicsManager.addEntity(entity.get(), std::move(rb));

		// Collider data
		auto terrainCollider = createTerrainCollider(heightMap, scaleVector);
		mCollisionManager.addEntity(entity.get(), std::move(terrainCollider));

		return entity;
	}

// Private functions
	TerrainLoader::RenderableTerrainUPtr TerrainLoader::createTerrainRenderable(
		float size, float maxHeight,
		const Image& heightMap, const std::vector<float>& lodDistances
	) {
		auto heightMapTexture = std::make_shared<se::graphics::Texture>();
		heightMapTexture->setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
		heightMapTexture->setImage(
			heightMap.pixels.get(), se::graphics::TypeId::UnsignedByte, se::graphics::ColorFormat::Red,
			heightMap.width, heightMap.height
		);

		std::shared_ptr<se::graphics::Material> terrainMaterial(new se::graphics::Material{
			"terrain_material",
			se::graphics::PBRMetallicRoughness{ { 0.5f, 0.25f, 0.1f, 1.0f }, nullptr, 0.2f, 0.5f, nullptr },
			nullptr, 1.0f, nullptr, 1.0f, nullptr, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
		});

		return std::make_unique<se::graphics::RenderableTerrain>(size, maxHeight, heightMapTexture, lodDistances, terrainMaterial);
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
