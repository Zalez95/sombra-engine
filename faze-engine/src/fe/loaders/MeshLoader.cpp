#include <glm/gtc/type_ptr.hpp>
#include "fe/loaders/MeshLoader.h"
#include "fe/loaders/RawMesh.h"
#include "fe/graphics/3D/Mesh.h"
#include "fe/graphics/buffers/VertexBuffer.h"
#include "fe/graphics/buffers/IndexBuffer.h"
#include "fe/graphics/buffers/VertexArray.h"

namespace fe { namespace loaders {

	MeshLoader::MeshUPtr MeshLoader::createMesh(const RawMesh& rawMesh) const
	{
		MeshUPtr graphicsMesh = nullptr;

		auto vao = std::make_unique<graphics::VertexArray>();
		std::vector<std::unique_ptr<graphics::VertexBuffer>> vbos;
		if (!rawMesh.positions.empty()) {
			auto vbo = std::make_unique<graphics::VertexBuffer>(
				glm::value_ptr(rawMesh.positions.front()),
				3 * rawMesh.positions.size(),
				3
			);

			vao->addBuffer(vbo.get(), Attributes::POSITION_ATTRIBUTE);
			vbos.push_back(std::move(vbo));
		}

		if (!rawMesh.normals.empty()) {
			auto vbo = std::make_unique<graphics::VertexBuffer>(
				glm::value_ptr(rawMesh.normals.front()),
				3 * rawMesh.normals.size(),
				3
			);

			vao->addBuffer(vbo.get(), Attributes::NORMAL_ATTRIBUTE);
			vbos.push_back(std::move(vbo));
		}

		if (!rawMesh.uvs.empty()) {
			auto vbo = std::make_unique<graphics::VertexBuffer>(
				glm::value_ptr(rawMesh.uvs.front()),
				2 * rawMesh.uvs.size(),
				2
			);

			vao->addBuffer(vbo.get(), Attributes::UV_ATTRIBUTE);
			vbos.push_back(std::move(vbo));
		}

		if (!rawMesh.jointWeights.empty()) {
			auto vbo = std::make_unique<graphics::VertexBuffer>(rawMesh.jointWeights.data(), rawMesh.jointWeights.size(), 4);
			vao->addBuffer(vbo.get(), Attributes::JOINT_WEIGHT_ATTRIBUTE);
			vbos.push_back(std::move(vbo));
		}

		if (!rawMesh.jointIndices.empty()) {
			auto vbo = std::make_unique<graphics::VertexBuffer>(rawMesh.jointIndices.data(), rawMesh.jointIndices.size(), 4);
			vao->addBuffer(vbo.get(), Attributes::JOINT_INDEX_ATTRIBUTE);
			vbos.push_back(std::move(vbo));
		}

		if (!rawMesh.faceIndices.empty()) {
			auto ibo = std::make_unique<graphics::IndexBuffer>(rawMesh.faceIndices.data(), rawMesh.faceIndices.size());
			vao->bind();
			ibo->bind();
			vao->unbind();

			graphicsMesh = std::make_unique<graphics::Mesh>(rawMesh.name, std::move(vbos), std::move(ibo), std::move(vao));
		}

		return graphicsMesh;
	}

}}
