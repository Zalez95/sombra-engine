#include <cassert>
#include <glm/gtc/matrix_transform.hpp>
#include "fe/loaders/RawMesh.h"
#include "fe/loaders/TerrainLoader.h"
#include "fe/app/Entity.h"
#include "fe/utils/Image.h"
#include "fe/graphics/3D/Mesh.h"
#include "fe/graphics/3D/Renderable3D.h"
#include "fe/physics/PhysicsEntity.h"
#include "fe/collision/TerrainCollider.h"

namespace fe { namespace loaders {

	TerrainLoader::EntityUPtr TerrainLoader::createTerrain(
		const std::string& name, float size,
		const utils::Image& heightMap, float maxHeight
	) {
		auto rawMesh = createRawMesh(name, heightMap);
		glm::mat4 transforms = glm::scale(glm::mat4(1.0f), glm::vec3(size, maxHeight, size));

		auto entity = std::make_unique<app::Entity>(name);

		// Graphics data
		auto graphicsMesh = mMeshLoader.createMesh(*rawMesh);
		auto renderable3D = std::make_unique<graphics::Renderable3D>(std::move(graphicsMesh), nullptr, nullptr);
		mGraphicsManager.addEntity(entity.get(), std::move(renderable3D), transforms);

		// Physics data
		physics::RigidBody rb;
		auto terrainCollider = createTerrainCollider(heightMap);
		auto physicsEntity = std::make_unique<physics::PhysicsEntity>(rb, std::move(terrainCollider), transforms);
		mPhysicsManager.addEntity(entity.get(), std::move(physicsEntity));

		return entity;
	}

// Private functions
	TerrainLoader::RawMeshUPtr TerrainLoader::createRawMesh(
		const std::string& name, const utils::Image& heightMap
	) const
	{
		// Get the data from the image
		const size_t xSize	= heightMap.getWidth();
		const size_t zSize	= heightMap.getHeight();
		const size_t count	= xSize * zSize;

		// The mesh data of the Terrain
		auto rawMesh = std::make_unique<RawMesh>(name);
		rawMesh->positions.reserve(3 * count);
		rawMesh->normals.reserve(3 * count);
		rawMesh->uvs.reserve(2 * count);
		rawMesh->faceIndices.reserve(6 * (xSize - 1) * (zSize - 1));

		for (size_t z = 0; z < zSize; ++z) {
			float zPos = z / static_cast<float>(zSize - 1) - 0.5f;
			for (size_t x = 0; x < xSize; ++x) {
				float xPos = x / static_cast<float>(xSize - 1) - 0.5f;
				float yPos = getHeight(heightMap, x, z);

				// Set the position
				rawMesh->positions.emplace_back(xPos, yPos, zPos);

				// Set the uvs
				rawMesh->uvs.emplace_back(static_cast<float>(x) / xSize, static_cast<float>(z) / zSize);

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

					// Set the normal vector of the current vertex
					rawMesh->normals.insert(rawMesh->normals.end(), { normal.x, normal.y, normal.z });

					// update the normals of the other vertices
					rawMesh->normals[topLeft]		+= n1;
					rawMesh->normals[bottomLeft]	+= normal;
					rawMesh->normals[bottomRight]	+= n2;

					// Normalize the normal of the bottom left vertex
					int norm = 6;
					if (x == 1) { norm -= 3; }
					if (z == 1) { norm -= 3; }
					if (norm > 0) {
						rawMesh->normals[bottomLeft] /= norm;
					}

					// Set the indices of the faces
					rawMesh->faceIndices.insert(rawMesh->faceIndices.end(), { topRight, bottomLeft, topLeft, topRight, bottomRight, bottomLeft });
				}
				else {
					rawMesh->normals.emplace_back(0.0f);
				}
			}
		}

		return rawMesh;
	}


	TerrainLoader::TerrainColliderUPtr TerrainLoader::createTerrainCollider(
		const utils::Image& heightMap
	) const
	{
		const size_t xSize	= heightMap.getWidth();
		const size_t zSize	= heightMap.getHeight();

		std::vector<float> heights;
		heights.reserve(xSize * zSize);
		for (size_t z = 0; z < zSize; ++z) {
			for (size_t x = 0; x < xSize; ++x) {
				heights.push_back( getHeight(heightMap, x, z) );
			}
		}

		return std::make_unique<collision::TerrainCollider>(heights, xSize, zSize);
	}


	float TerrainLoader::getHeight(const utils::Image& heightMap, size_t x, size_t z) const
	{
		assert(x < heightMap.getWidth() && "x must be smaller than the image width");
		assert(z < heightMap.getHeight() && "z must be smaller than the image height");

		unsigned char* heightMapPixels = heightMap.getPixels();
		unsigned char l = heightMapPixels[z * heightMap.getWidth() + x];

		return (l / kMaxColor - 0.5f);
	}

}}
