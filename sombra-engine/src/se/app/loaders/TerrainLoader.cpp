#include <cassert>
#include "se/utils/Repository.h"
#include "se/graphics/core/Program.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/Pass.h"
#include "se/graphics/Technique.h"
#include "se/graphics/Renderer.h"
#include "se/graphics/GraphicsEngine.h"
#include "se/graphics/3D/RenderableTerrain.h"
#include "se/physics/RigidBody.h"
#include "se/collision/TerrainCollider.h"
#include "se/app/EntityDatabase.h"
#include "se/app/Scene.h"
#include "se/app/TagComponent.h"
#include "se/app/TransformsComponent.h"
#include "se/app/loaders/TerrainLoader.h"
#include "se/app/graphics/Material.h"
#include "se/app/graphics/TextureUtils.h"

namespace se::app {

	Entity TerrainLoader::createTerrain(
		const char* name, float size, float maxHeight, const Image<unsigned char>& heightMap,
		const std::vector<float>& lodDistances, const char* techniqueName
	) {
		glm::vec3 scaleVector(size, 2.0f * maxHeight, size);

		// Entity
		auto entity = mEntityDatabase.addEntity();

		// Name
		TagComponent tag(name);
		mEntityDatabase.addComponent(entity, std::move(tag));

		// Transforms
		TransformsComponent transforms;
		transforms.scale = scaleVector;
		mEntityDatabase.addComponent(entity, std::move(transforms));

		// Graphics data
		auto renderableTerrain = createTerrainRenderable(size, maxHeight, heightMap, lodDistances, techniqueName);
		mEntityDatabase.addComponent(entity, std::move(renderableTerrain));

		// Physics data
		physics::RigidBodyConfig config(0.2f);
		config.frictionCoefficient = 1.0f;
		physics::RigidBody rb(config, physics::RigidBodyData());
		mEntityDatabase.addComponent(entity, std::move(rb));

		// Collider data
		auto terrainCollider = createTerrainCollider(heightMap, scaleVector);
		mEntityDatabase.addComponent(entity, std::move(terrainCollider));

		mScene.entities.push_back(entity);
		return entity;
	}

// Private functions
	graphics::RenderableTerrain TerrainLoader::createTerrainRenderable(
		float size, float maxHeight,
		const Image<unsigned char>& heightMap, const std::vector<float>& lodDistances,
		const char* techniqueName
	) {
		graphics::RenderableTerrain renderable(size, lodDistances);

		auto heightMapTexture = std::make_shared<graphics::Texture>(graphics::TextureTarget::Texture2D);
		heightMapTexture->setTextureUnit(SplatmapMaterial::TextureUnits::kHeightMap)
			.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear)
			.setWrapping(se::graphics::TextureWrap::ClampToEdge, se::graphics::TextureWrap::ClampToEdge)
			.setImage(
				heightMap.pixels.get(), graphics::TypeId::UnsignedByte, graphics::ColorFormat::Red, graphics::ColorFormat::Red,
				heightMap.width, heightMap.height
			);

		auto normalMapTexture = TextureUtils::heightmapToNormalMapLocal(heightMapTexture, heightMap.width, heightMap.height);
		normalMapTexture->setTextureUnit(SplatmapMaterial::TextureUnits::kNormalMap);

		auto technique = mScene.repository.find<std::string, graphics::Technique>(techniqueName);
		if (technique) {
			std::shared_ptr<graphics::Program> program;
			technique->processPasses([&](auto& pass) {
				pass->processBindables([&](const auto& bindable) {
					if (auto tmp = std::dynamic_pointer_cast<graphics::Program>(bindable)) {
						program = tmp;
					}
				});
			});

			renderable.addBindable(std::move(heightMapTexture))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
					"uHeightMap", *program, SplatmapMaterial::TextureUnits::kHeightMap
				))
				.addBindable(std::move(normalMapTexture))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>(
					"uNormalMap", *program, SplatmapMaterial::TextureUnits::kNormalMap
				))
				.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
					"uXZSize", *program, size
				))
				.addBindable(std::make_shared<graphics::UniformVariableValue<float>>(
					"uMaxHeight", *program, maxHeight
				))
				.addTechnique(technique);
		}

		return renderable;
	}


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
