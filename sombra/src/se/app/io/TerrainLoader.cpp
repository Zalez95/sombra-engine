#include <cassert>
#include "se/utils/Repository.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/Renderer.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/physics/RigidBody.h"
#include "se/collision/TerrainCollider.h"
#include "se/app/EntityDatabase.h"
#include "se/app/Scene.h"
#include "se/app/TagComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/TerrainComponent.h"
#include "se/app/io/TerrainLoader.h"
#include "se/app/graphics/Material.h"
#include "se/app/graphics/TextureUtils.h"

namespace se::app {

	Entity TerrainLoader::createTerrain(
		const char* name, float size, float maxHeight, const Image<unsigned char>& heightMap,
		const std::vector<float>& lodDistances, const char* shaderName
	) {
		glm::vec3 scaleVector(size, 2.0f * maxHeight, size);

		// Entity
		auto entity = mEntityDatabase.addEntity();

		// Name
		mEntityDatabase.emplaceComponent<TagComponent>(entity, name);

		// Transforms
		TransformsComponent transforms;
		transforms.scale = scaleVector;
		mEntityDatabase.addComponent(entity, std::move(transforms));

		// Graphics data
		auto terrain = mEntityDatabase.emplaceComponent<TerrainComponent>(
			entity, mEventManager, entity, size, maxHeight, lodDistances
		);
		auto shader = mScene.repository.find<std::string, RenderableShader>(shaderName);
		if (shader) {
			terrain->addRenderableShader(shader);
		}

		// Physics data
		physics::RigidBodyConfig config;
		config.frictionCoefficient = 1.0f;
		config.sleepMotion = 0.2f;
		mEntityDatabase.emplaceComponent<physics::RigidBody>(entity, config);

		// Collider data
		mEntityDatabase.addComponent(entity, createTerrainCollider(heightMap, scaleVector));

		mScene.entities.push_back(entity);
		return entity;
	}

// Private functions
	TerrainLoader::ColliderUPtr TerrainLoader::createTerrainCollider(
		const Image<unsigned char>& heightMap, const glm::vec3& scaleVector
	) {
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


	float TerrainLoader::getHeight(const Image<unsigned char>& heightMap, std::size_t x, std::size_t z)
	{
		assert(x < heightMap.width && "x must be smaller than the image width");
		assert(z < heightMap.height && "z must be smaller than the image height");

		unsigned char* heightMapPixels = static_cast<unsigned char*>(heightMap.pixels.get());
		unsigned char h = heightMapPixels[z * heightMap.width + x];

		return h / static_cast<float>(kMaxColor) - 0.5f;
	}

}
