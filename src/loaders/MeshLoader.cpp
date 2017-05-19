#include "MeshLoader.h"
#include <map>
#include <string>
#include <sstream>
#include <glm/glm.hpp>
#include "../graphics/3D/Mesh.h"
#include "../graphics/buffers/VertexBuffer.h"
#include "../graphics/buffers/IndexBuffer.h"
#include "../graphics/buffers/VertexArray.h"

namespace graphics {

// Public Functions
	MeshLoader::MeshUPtr MeshLoader::createMesh(
		const std::string& name,
		const std::vector<GLfloat>& positions,
		const std::vector<GLfloat>& normals,
		const std::vector<GLfloat>& uvs,
		const std::vector<GLushort>& faceIndices
	) {
		auto vao = std::make_unique<VertexArray>();
		auto ibo = std::make_unique<IndexBuffer>(faceIndices.data(), faceIndices.size());
		std::vector<std::unique_ptr<VertexBuffer>> vbos;
		std::unique_ptr<VertexBuffer> tmp;

		tmp = std::make_unique<VertexBuffer>(positions.data(), positions.size(), 3);
		vao->addBuffer(tmp.get(), POSITION_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<VertexBuffer>(normals.data(), normals.size(), 3);
		vao->addBuffer(tmp.get(), NORMAL_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<VertexBuffer>(uvs.data(), uvs.size(), 2);
		vao->addBuffer(tmp.get(), UV_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		vao->bind();
		ibo->bind();
		vao->unbind();

		return std::make_unique<Mesh>(name, std::move(vbos), std::move(ibo), std::move(vao));
	}


	MeshLoader::MeshUPtr MeshLoader::createMesh(
		const std::string& name,
		const std::vector<GLfloat>& positions,
		const std::vector<GLfloat>& normals,
		const std::vector<GLfloat>& uvs,
		const std::vector<GLfloat>& jointWeights,
		const std::vector<GLushort>& jointIndices,
		const std::vector<GLushort>& faceIndices
	) {
		std::vector<std::unique_ptr<VertexBuffer>> vbos;
		auto ibo = std::make_unique<IndexBuffer>(faceIndices.data(), faceIndices.size());
		auto vao = std::make_unique<VertexArray>();

		std::unique_ptr<VertexBuffer> tmp;
		tmp = std::make_unique<VertexBuffer>(positions.data(), positions.size(), 3);
		vao->addBuffer(tmp.get(), POSITION_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<VertexBuffer>(normals.data(), normals.size(), 3);
		vao->addBuffer(tmp.get(), NORMAL_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<VertexBuffer>(uvs.data(), uvs.size(), 2);
		vao->addBuffer(tmp.get(), UV_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<VertexBuffer>(jointWeights.data(), jointWeights.size(), 4);
		vao->addBuffer(tmp.get(), JOINT_WEIGHT_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<VertexBuffer>(jointIndices.data(), jointIndices.size(), 4);
		vao->addBuffer(tmp.get(), JOINT_INDEX_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		vao->bind();
		ibo->bind();
		vao->unbind();

		return std::make_unique<Mesh>(name, std::move(vbos), std::move(ibo), std::move(vao));
	}


	std::vector<GLfloat> MeshLoader::calculateNormals(
		const std::vector<GLfloat>& positions,
		const std::vector<GLushort>& faceIndices
	) const
	{
		std::vector<GLfloat> normals(positions.size(), 0);

		// Sum to the normal of every vertex, the normal of the faces 
		// which it belongs
		for (unsigned int i = 0; i < faceIndices.size(); i+=3) {
			// Get the normal of triangle
			GLfloat v1_x = positions[3 * faceIndices[i]]		- positions[3 * faceIndices[i+1]];
			GLfloat v1_y = positions[3 * faceIndices[i] + 1]	- positions[3 * faceIndices[i+1] + 1];
			GLfloat v1_z = positions[3 * faceIndices[i] + 2]	- positions[3 * faceIndices[i+1] + 2];
			glm::vec3 v1(v1_x, v1_y, v1_z);

			GLfloat v2_x = positions[3 * faceIndices[i]]		- positions[3 * faceIndices[i+2]];
			GLfloat v2_y = positions[3 * faceIndices[i] + 1]	- positions[3 * faceIndices[i+2] + 1];
			GLfloat v2_z = positions[3 * faceIndices[i] + 2]	- positions[3 * faceIndices[i+2] + 2];
			glm::vec3 v2(v2_x, v2_y, v1_z);

			glm::vec3 normal = glm::cross(v1, v2);

			normals[3 * faceIndices[i]]			+= normal.x;
			normals[3 * faceIndices[i] + 1]		+= normal.y;
			normals[3 * faceIndices[i] + 2]		+= normal.z;

			normals[3 * faceIndices[i+1]]		+= normal.x;
			normals[3 * faceIndices[i+1] + 1]	+= normal.y;
			normals[3 * faceIndices[i+1] + 2]	+= normal.z;

			normals[3 * faceIndices[i+2]]		+= normal.x;
			normals[3 * faceIndices[i+2] + 1]	+= normal.y;
			normals[3 * faceIndices[i+2] + 2]	+= normal.z;
		}

		// Normalize the normal vector of every vertex
		for (unsigned int i = 0; i < normals.size(); i+=3) {
			GLfloat length	= sqrt( pow(normals[i], 2) + pow(normals[i+1], 2) + pow(normals[i+2], 2) );
			normals[i]		/= length;
			normals[i+1]	/= length;
			normals[i+2]	/= length;
		}

		return normals;
	}

}
