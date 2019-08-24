#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include "se/loaders/RawMesh.h"
#include "se/loaders/TerrainLoader.h"
#include "se/loaders/MeshLoader.h"
#include "se/app/Entity.h"
#include "se/utils/Image.h"
#include "se/graphics/3D/Mesh.h"
#include "se/graphics/3D/Material.h"
#include "se/graphics/3D/Renderable3D.h"
#include "se/physics/RigidBody.h"
#include "se/collision/TerrainCollider.h"

namespace se::loaders {

	TerrainLoader::EntityUPtr TerrainLoader::createTerrain(
		const std::string& name, float size,
		const utils::Image& heightMap, float maxHeight
	) {
		auto rawMesh = createRawMesh(name, heightMap);
		glm::vec3 scaleVector(size, maxHeight, size);

		// Entity
		auto entity = std::make_unique<app::Entity>(name);
		entity->scale = scaleVector;

		// Graphics data
		auto graphicsMesh = std::make_shared<graphics::Mesh>( MeshLoader::createGraphicsMesh(*rawMesh) );
		auto graphicsMaterial = std::make_shared<graphics::Material>(graphics::Material{
			"default",
			{ glm::vec4(0.5f, 0.5f, 0.5f, 1.0f), nullptr, 1.0f, 1.0f, nullptr },
			nullptr, nullptr, nullptr, glm::vec3(0.0f), graphics::AlphaMode::Opaque, 0.0f, false
		});
		auto renderable3D = std::make_unique<graphics::Renderable3D>(graphicsMesh, graphicsMaterial);
		mGraphicsManager.addEntity(entity.get(), std::move(renderable3D));

		// Physics data
		auto rb = std::make_unique<physics::RigidBody>(physics::RigidBodyConfig(0.2f), physics::RigidBodyData());
		mPhysicsManager.addEntity(entity.get(), std::move(rb));

		// Collider data
		glm::mat4 scale = glm::scale(glm::mat4(1.0f), scaleVector);
		auto terrainCollider = createTerrainCollider(heightMap);
		terrainCollider->setTransforms(scale);
		mCollisionManager.addEntity(entity.get(), std::move(terrainCollider));

		return entity;
	}

// Private functions
	TerrainLoader::RawMeshUPtr TerrainLoader::createRawMesh(
		const std::string& name, const utils::Image& heightMap
	) const
	{
		// Get the data from the image
		const std::size_t xSize = heightMap.width;
		const std::size_t zSize = heightMap.height;
		const std::size_t count = xSize * zSize;

		// The mesh data of the Terrain
		auto rawMesh = std::make_unique<RawMesh>(name);
		rawMesh->positions.reserve(count);
		rawMesh->normals.reserve(count);
		rawMesh->tangents.reserve(count);
		rawMesh->texCoords.reserve(count);
		rawMesh->faceIndices.reserve(6 * (xSize - 1) * (zSize - 1));

		for (std::size_t z = 0; z < zSize; ++z) {
			float zPos = z / static_cast<float>(zSize - 1) - 0.5f;
			for (std::size_t x = 0; x < xSize; ++x) {
				float xPos = x / static_cast<float>(xSize - 1) - 0.5f;
				float yPos = getHeight(heightMap, x, z);

				// Set the position
				rawMesh->positions.emplace_back(xPos, yPos, zPos);

				// Set the texCoords
				rawMesh->texCoords.emplace_back(static_cast<float>(x) / xSize, static_cast<float>(z) / zSize);

				if ((x > 0) && (z > 0)) {
					// Calculate the indices of the vertices that creates the faces
					unsigned short topRight		= static_cast<unsigned short>(z * xSize + x);
					unsigned short topLeft		= static_cast<unsigned short>(topRight - 1);
					unsigned short bottomRight	= static_cast<unsigned short>((z - 1) * xSize + x);
					unsigned short bottomLeft	= static_cast<unsigned short>(bottomRight - 1);

					// Calculate the normals of the faces
					glm::vec3 tr( xPos, yPos, zPos );
					const glm::vec3& tl = rawMesh->positions[topLeft];
					const glm::vec3& br = rawMesh->positions[bottomRight];
					const glm::vec3& bl = rawMesh->positions[bottomLeft];

					glm::vec3 n1 = glm::cross(tr - tl, tr - bl);
					glm::vec3 n2 = glm::cross(tr - bl, tr - br);
					glm::vec3 normal = n1 + n2;

					// Calculate the tangents of the faces
					glm::vec3 t1 = tr - tl;
					glm::vec3 t2 = br - bl;
					glm::vec3 tangent = t1 + t2;

					// Set the normal and tangent vectors of the vertices
					rawMesh->normals.emplace_back(normal.x, normal.y, normal.z);
					rawMesh->normals[topLeft]		+= n1;
					rawMesh->normals[bottomLeft]	+= normal;
					rawMesh->normals[bottomRight]	+= n2;
					rawMesh->tangents.emplace_back(tangent.x, tangent.y, tangent.z);
					rawMesh->tangents[topLeft]		+= t1;
					rawMesh->tangents[bottomLeft]	+= tangent;
					rawMesh->tangents[bottomRight]	+= t2;

					// Normalize the vectors of the bottom left vertex
					int norm = 6;
					if (x == 1) { norm -= 3; }
					if (z == 1) { norm -= 3; }
					if (norm > 0) {
						rawMesh->normals[bottomLeft] /= norm;
						rawMesh->tangents[bottomLeft] /= norm;
					}

					// Set the indices of the faces
					rawMesh->faceIndices.insert(rawMesh->faceIndices.end(), { topRight, bottomLeft, topLeft, topRight, bottomRight, bottomLeft });
				}
				else {
					rawMesh->normals.emplace_back(0.0f);
					rawMesh->tangents.emplace_back(0.0f);
				}
			}
		}

		return rawMesh;
	}


	TerrainLoader::TerrainColliderUPtr TerrainLoader::createTerrainCollider(
		const utils::Image& heightMap
	) const
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

		return std::make_unique<collision::TerrainCollider>(heights, xSize, zSize);
	}


	float TerrainLoader::getHeight(const utils::Image& heightMap, std::size_t x, std::size_t z) const
	{
		assert(x < heightMap.width && "x must be smaller than the image width");
		assert(z < heightMap.height && "z must be smaller than the image height");

		std::byte* heightMapPixels = heightMap.pixels.get();
		std::byte h = heightMapPixels[z * heightMap.width + x];

		return std::to_integer<unsigned char>(h) / static_cast<float>(kMaxColor) - 0.5f;
	}

}
