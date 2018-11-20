#include <glm/gtc/type_ptr.hpp>
#include "se/loaders/MeshLoader.h"
#include "se/loaders/RawMesh.h"
#include "se/graphics/3D/Mesh.h"
#include "se/graphics/buffers/VertexBuffer.h"
#include "se/graphics/buffers/IndexBuffer.h"
#include "se/graphics/buffers/VertexArray.h"
#include "se/collision/HalfEdgeMesh.h"
#include "se/collision/HalfEdgeMeshExt.h"

namespace se::loaders {

	MeshLoader::GraphicsMeshUPtr MeshLoader::createGraphicsMesh(const RawMesh& rawMesh)
	{
		GraphicsMeshUPtr graphicsMesh;

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


	MeshLoader::HalfEdgeMeshUPtr MeshLoader::createHalfEdgeMesh(const RawMesh& rawMesh)
	{
		auto heMesh = std::make_unique<collision::HalfEdgeMesh>();

		// Add the HEVertices
		std::map<int, int> vertexMap;
		for (std::size_t iVertex1 = 0; iVertex1 < rawMesh.positions.size(); ++iVertex1) {
			int iVertex2 = collision::addVertex(*heMesh, rawMesh.positions[iVertex1]);
			vertexMap.emplace(iVertex1, iVertex2);
		}

		// Add the HEFaces
		for (std::size_t i = 0; i < rawMesh.faceIndices.size(); i += 3) {
			int iFace = collision::addFace(
				*heMesh,
				{
					vertexMap[ rawMesh.faceIndices[i] ],
					vertexMap[ rawMesh.faceIndices[i+1] ],
					vertexMap[ rawMesh.faceIndices[i+2] ]
				}
			);

			if (iFace < 0) {
				return nullptr;
			}
		}

		// Validate the HEMesh
		if (!collision::validateMesh(*heMesh).first) {
			return nullptr;
		}

		return heMesh;
	}

}
