#include "fe/loaders/TerrainLoader.h"
#include <cassert>
#include <glm/gtc/type_ptr.hpp>
#include "fe/app/Entity.h"
#include "fe/utils/Image.h"
#include "fe/graphics/3D/Mesh.h"
#include "fe/graphics/3D/Renderable3D.h"
#include "fe/loaders/RawMesh.h"

namespace fe { namespace loaders {

// Static variables definition
	const float TerrainLoader::MAX_COLOR = 255.0f;

// Public functions
	TerrainLoader::EntityUPtr TerrainLoader::createTerrain(
		const std::string& name, float size,
		const utils::Image& heightMap, float maxHeight
	) {
		auto rawMesh = createRawMesh(name, size, heightMap, maxHeight);

		auto entity = std::make_unique<app::Entity>(name);

		// Graphics data
		auto graphicsMesh = mMeshLoader.createMesh(*rawMesh);
		auto renderable3D = std::make_unique<graphics::Renderable3D>(std::move(graphicsMesh), nullptr, nullptr);
		mGraphicsManager.addEntity(entity.get(), std::move(renderable3D), glm::mat4());

		// Physics data
		// TODO

		return entity;
	}

// Private functions
	TerrainLoader::RawMeshUPtr TerrainLoader::createRawMesh(
		const std::string& name, float size,
		const utils::Image& heightMap, float maxHeight
	) const
	{
		// Get the data from the image
		const size_t width	= heightMap.getWidth();
		const size_t height	= heightMap.getHeight();
		const size_t count	= width * height;

		// The mesh data of the Terrain
		auto rawMesh = std::make_unique<RawMesh>(name);
		rawMesh->mPositions.reserve(3 * count);
		rawMesh->mNormals.reserve(3 * count);
		rawMesh->mUVs.reserve(2 * count);
		rawMesh->mFaceIndices.reserve(6 * (width - 1) * (height - 1));

		for (size_t i = 0; i < height; ++i) {
			float zPos = (i / static_cast<float>(height - 1) - 0.5f) * size;
			for (size_t j = 0; j < width; ++j) {
				float xPos = (j / static_cast<float>(width - 1) - 0.5f) * size;
				float yPos = getHeight(heightMap, maxHeight, j, i);

				// Set the position
				rawMesh->mPositions.emplace_back(xPos, yPos, zPos);

				// Set the uvs
				rawMesh->mUVs.emplace_back(j / static_cast<float>(width), i / static_cast<float>(height));

				if ((i > 0) && (j > 0)) {
					// Calculate the indices of the vertices that creates the faces
					unsigned short topRight		= static_cast<unsigned short>(i * width + j);
					unsigned short topLeft		= static_cast<unsigned short>(topRight - 1);
					unsigned short bottomRight	= static_cast<unsigned short>((i - 1) * width + j);
					unsigned short bottomLeft	= static_cast<unsigned short>(bottomRight - 1);

					// Calculate the normals of the faces
					glm::vec3 tr( xPos, yPos, zPos );
					const glm::vec3& tl = rawMesh->mPositions[topLeft];
					const glm::vec3& br = rawMesh->mPositions[bottomRight];
					const glm::vec3& bl = rawMesh->mPositions[bottomLeft];

					glm::vec3 n1 = glm::cross(tr - tl, tr - bl);
					glm::vec3 n2 = glm::cross(tr - bl, tr - br);
					glm::vec3 normal = n1 + n2;

					// Set the normal vector of the current vertex
					rawMesh->mNormals.insert(rawMesh->mNormals.end(), { normal.x, normal.y, normal.z });

					// update the normals of the other vertices
					rawMesh->mNormals[topLeft]		+= n1;
					rawMesh->mNormals[bottomLeft]	+= normal;
					rawMesh->mNormals[bottomRight]	+= n2;

					// Normalize the normal of the bottom left vertex
					int norm = 6;
					if (i == 1) { norm -= 3; }
					if (j == 1) { norm -= 3; }
					if (norm > 0) {
						rawMesh->mNormals[bottomLeft] /= norm;
					}

					// Set the indices of the faces
					rawMesh->mFaceIndices.insert(rawMesh->mFaceIndices.end(), { topRight, bottomLeft, topLeft, topRight, bottomRight, bottomLeft });
				}
				else {
					rawMesh->mNormals.emplace_back(0.0f);
				}
			}
		}

		return rawMesh;
	}


	float TerrainLoader::getHeight(
		const utils::Image& heightMap, float maxHeight,
		size_t x, size_t z
	) const
	{
		assert(x < heightMap.getWidth() && "x must be smaller than the image width");
		assert(z < heightMap.getHeight() && "z must be smaller than the image height");

		unsigned char* heightMapPixels = heightMap.getPixels();
		unsigned char l = heightMapPixels[z * heightMap.getWidth() + x];

		return (l / MAX_COLOR - 0.5f) * maxHeight;
	}

}}
