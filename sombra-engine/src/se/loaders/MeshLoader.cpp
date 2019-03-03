#include <glm/gtc/type_ptr.hpp>
#include "se/loaders/MeshLoader.h"
#include "se/loaders/RawMesh.h"
#include "se/graphics/buffers/VertexBuffer.h"
#include "se/graphics/buffers/IndexBuffer.h"
#include "se/graphics/buffers/VertexArray.h"
#include "se/collision/HalfEdgeMeshExt.h"

namespace se::loaders {

	graphics::Mesh MeshLoader::createGraphicsMesh(const RawMesh& rawMesh)
	{
		using namespace graphics;

		VertexArray vao;
		std::vector<VertexBuffer> vbos;

		if (!rawMesh.positions.empty()) {
			auto& vbo = vbos.emplace_back(glm::value_ptr(rawMesh.positions.front()), 3 * rawMesh.positions.size());
			vao.bind();
			vbo.bind();
			vao.setVertexAttribute(static_cast<unsigned int>(MeshAttributes::PositionAttribute), TypeId::Float, false, 3, 0);
			vao.unbind();
		}

		if (!rawMesh.normals.empty()) {
			auto& vbo = vbos.emplace_back(glm::value_ptr(rawMesh.normals.front()), 3 * rawMesh.normals.size());
			vao.bind();
			vbo.bind();
			vao.setVertexAttribute(static_cast<unsigned int>(MeshAttributes::NormalAttribute), TypeId::Float, false, 3, 0);
			vao.unbind();
		}

		if (!rawMesh.uvs.empty()) {
			auto& vbo = vbos.emplace_back(glm::value_ptr(rawMesh.uvs.front()), 2 * rawMesh.uvs.size());
			vao.bind();
			vbo.bind();
			vao.setVertexAttribute(static_cast<unsigned int>(MeshAttributes::UVAttribute), TypeId::Float, false, 2, 0);
			vao.unbind();
		}

		if (!rawMesh.jointWeights.empty()) {
			auto& vbo = vbos.emplace_back(rawMesh.jointWeights.data(), rawMesh.jointWeights.size());
			vao.bind();
			vbo.bind();
			vao.setVertexAttribute(static_cast<unsigned int>(MeshAttributes::JointWeightAttribute), TypeId::Float, false, 4, 0);
			vao.unbind();
		}

		if (!rawMesh.jointIndices.empty()) {
			auto& vbo = vbos.emplace_back(rawMesh.jointIndices.data(), rawMesh.jointIndices.size());
			vao.bind();
			vbo.bind();
			vao.setVertexAttribute(static_cast<unsigned int>(MeshAttributes::JointIndexAttribute), TypeId::Float, false, 4, 0);
			vao.unbind();
		}

		IndexBuffer ibo(rawMesh.faceIndices.data(), TypeId::UnsignedShort, rawMesh.faceIndices.size());
		vao.bind();
		ibo.bind();
		vao.unbind();

		return Mesh(std::move(vbos), std::move(ibo), std::move(vao));
	}


	std::pair<collision::HalfEdgeMesh, bool> MeshLoader::createHalfEdgeMesh(const RawMesh& rawMesh)
	{
		collision::HalfEdgeMesh heMesh;

		// Add the HEVertices
		std::map<int, int> vertexMap;
		for (int iVertex1 = 0; iVertex1 < static_cast<int>(rawMesh.positions.size()); ++iVertex1) {
			int iVertex2 = collision::addVertex(heMesh, rawMesh.positions[iVertex1]);
			vertexMap.emplace(iVertex1, iVertex2);
		}

		// Add the HEFaces
		bool allFacesLoaded = true;
		for (std::size_t i = 0; i < rawMesh.faceIndices.size(); i += 3) {
			int iFace = collision::addFace(
				heMesh,
				{
					vertexMap[ rawMesh.faceIndices[i] ],
					vertexMap[ rawMesh.faceIndices[i+1] ],
					vertexMap[ rawMesh.faceIndices[i+2] ]
				}
			);

			if (iFace < 0) {
				allFacesLoaded = false;
			}
		}

		// Validate the HEMesh
		return std::pair(heMesh, allFacesLoaded && collision::validateMesh(heMesh).first);
	}

}
