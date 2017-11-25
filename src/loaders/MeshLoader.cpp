#include "MeshLoader.h"
#include <glm/gtc/type_ptr.hpp>
#include "../graphics/3D/Mesh.h"
#include "../graphics/buffers/VertexBuffer.h"
#include "../graphics/buffers/IndexBuffer.h"
#include "../graphics/buffers/VertexArray.h"
#include "RawMesh.h"

namespace loaders {

	MeshLoader::MeshUPtr MeshLoader::createMesh(const RawMesh& rawMesh) const
	{
		MeshUPtr graphicsMesh = nullptr;

		auto vao = std::make_unique<graphics::VertexArray>();
		std::vector<std::unique_ptr<graphics::VertexBuffer>> vbos;
		if (!rawMesh.mPositions.empty()) {
			auto vbo = std::make_unique<graphics::VertexBuffer>(
				glm::value_ptr(rawMesh.mPositions.front()),
				3 * rawMesh.mPositions.size(),
				3
			);

			vao->addBuffer(vbo.get(), POSITION_ATTRIBUTE);
			vbos.push_back(std::move(vbo));
		}

		if (!rawMesh.mNormals.empty()) {
			auto vbo = std::make_unique<graphics::VertexBuffer>(
				glm::value_ptr(rawMesh.mNormals.front()),
				3 * rawMesh.mNormals.size(),
				3
			);

			vao->addBuffer(vbo.get(), NORMAL_ATTRIBUTE);
			vbos.push_back(std::move(vbo));
		}

		if (!rawMesh.mUVs.empty()) {
			auto vbo = std::make_unique<graphics::VertexBuffer>(
				glm::value_ptr(rawMesh.mUVs.front()),
				2 * rawMesh.mUVs.size(),
				2
			);

			vao->addBuffer(vbo.get(), UV_ATTRIBUTE);
			vbos.push_back(std::move(vbo));
		}

		if (!rawMesh.mJointWeights.empty()) {
			auto vbo = std::make_unique<graphics::VertexBuffer>(rawMesh.mJointWeights.data(), rawMesh.mJointWeights.size(), 4);
			vao->addBuffer(vbo.get(), JOINT_WEIGHT_ATTRIBUTE);
			vbos.push_back(std::move(vbo));
		}

		if (!rawMesh.mJointIndices.empty()) {
			auto vbo = std::make_unique<graphics::VertexBuffer>(rawMesh.mJointIndices.data(), rawMesh.mJointIndices.size(), 4);
			vao->addBuffer(vbo.get(), JOINT_INDEX_ATTRIBUTE);
			vbos.push_back(std::move(vbo));
		}

		if (!rawMesh.mFaceIndices.empty()) {
			auto ibo = std::make_unique<graphics::IndexBuffer>(rawMesh.mFaceIndices.data(), rawMesh.mFaceIndices.size());
			vao->bind();
			ibo->bind();
			vao->unbind();

			graphicsMesh = std::make_unique<graphics::Mesh>(rawMesh.mName, std::move(vbos), std::move(ibo), std::move(vao));
		}

		return graphicsMesh;
	}

}
