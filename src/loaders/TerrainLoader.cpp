#include "TerrainLoader.h"
#include <cassert>
#include "../game/Entity.h"
#include "../utils/Image.h"
#include "../graphics/3D/Mesh.h"
#include "../graphics/3D/Renderable3D.h"
#include <iostream>
namespace loaders {

// Static variables definition
	const float TerrainLoader::MAX_COLOR = 255.0f;

// Public functions
	TerrainLoader::EntityUPtr TerrainLoader::createTerrain(
		std::string name, float size,
		const utils::Image& heightMap, float maxHeight
	) {
		auto terrain = std::make_unique<game::Entity>(name);

		auto mesh = createMesh(name, size, heightMap, maxHeight);
		auto renderable3D = std::make_unique<graphics::Renderable3D>(std::move(mesh), nullptr, nullptr);
		mGraphicsManager.addEntity(terrain.get(), std::move(renderable3D));

		return terrain;
	}

// Private functions
	TerrainLoader::MeshUPtr TerrainLoader::createMesh(
		const std::string& name, float size,
		const utils::Image& heightMap, float maxHeight
	) const
	{
		// Get the data from the image
		const size_t width	= heightMap.getWidth();
		const size_t height	= heightMap.getHeight();
		const size_t count	= width * height;

		// The mesh data of the Terrain
		std::vector<GLfloat> positions;		positions.reserve(3 * count);
		std::vector<GLfloat> normals;		normals.reserve(3 * count);
		std::vector<GLfloat> uvs;			uvs.reserve(2 * count);
		std::vector<GLushort> indices;		indices.reserve(6 * (width - 1) * (height - 1));

		for (size_t i = 0; i < height; ++i) {
			float zPos = (i / static_cast<float>(height - 1) - 0.5f) * size;
			for (size_t j = 0; j < width; ++j) {
				float xPos = (j / static_cast<float>(width - 1) - 0.5f) * size;
				float yPos = getHeight(heightMap, maxHeight, j, i);

				// Set the position
				positions.insert(positions.end(), { xPos, yPos, zPos });

				// Set the uvs
				uvs.insert(uvs.end(), {	j / static_cast<GLfloat>(width), i / static_cast<GLfloat>(height) });

				if ((i > 0) && (j > 0)) {
					// Calculate the indices of the vertices that creates the faces
					GLushort topRight		= i * width + j;
					GLushort topLeft		= topRight - 1;
					GLushort bottomRight	= (i - 1) * width + j;
					GLushort bottomLeft		= bottomRight - 1;

					// Calculate the normals of the faces
					glm::vec3 tr( xPos, yPos, zPos );
					glm::vec3 tl( positions[3 * topLeft], positions[3 * topLeft + 1], positions[3 * topLeft + 2] );
					glm::vec3 br( positions[3 * bottomRight], positions[3 * bottomRight + 1], positions[3 * bottomRight + 2] );
					glm::vec3 bl( positions[3 * bottomLeft], positions[3 * bottomLeft + 1], positions[3 * bottomLeft + 2] );

					glm::vec3 n1 = glm::cross(tr - tl, tr - bl);
					glm::vec3 n2 = glm::cross(tr - bl, tr - br);
					glm::vec3 normal = n1 + n2;

					// Set the normal vector of the current vertex
					normals.insert(normals.end(), { normal.x, normal.y, normal.z });

					// update the normals of the other vertices
					normals[3 * topLeft] += n1.x;			normals[3 * topLeft + 1] += n1.y;			normals[3 * topLeft + 2] += n1.z;
					normals[3 * bottomLeft] += normal.x;	normals[3 * bottomLeft + 1] += normal.y;	normals[3 * bottomLeft + 2] += normal.z;
					normals[3 * bottomRight] += n2.x;		normals[3 * bottomRight + 1] += n2.y;		normals[3 * bottomRight + 2] += n2.z;

					// Normalize the normal of the bottom left vertex
					int norm = 6;
					if (i == 1) { norm -= 3; }
					if (j == 1) { norm -= 3; }
					if (norm > 0) {
						normals[3 * bottomLeft] /= norm;	normals[3 * bottomLeft + 1] /= norm;		normals[3 * bottomLeft + 2] /= norm;
					}

					// Set the indices of the faces
					indices.insert(indices.end(), { topRight, bottomLeft, topLeft, topRight, bottomRight, bottomLeft });
				}
				else {
					normals.insert(normals.end(), { 0, 0, 0 });
				}
			}
		}

		return mMeshLoader.createMesh(name, positions, normals, uvs, indices);
	}

// Private functions
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

}
