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
		graphics::VertexArray vao;
		std::vector<graphics::VertexBuffer> vbos;

		if (!rawMesh.positions.empty()) {
			vbos.emplace_back(glm::value_ptr(rawMesh.positions.front()), 3 * rawMesh.positions.size());
			vao.addBuffer(Attributes::POSITION_ATTRIBUTE, vbos.back(), graphics::TypeId::Float, false, 3, 0);
		}

		if (!rawMesh.normals.empty()) {
			vbos.emplace_back(glm::value_ptr(rawMesh.normals.front()), 3 * rawMesh.normals.size());
			vao.addBuffer(Attributes::NORMAL_ATTRIBUTE, vbos.back(), graphics::TypeId::Float, false, 3, 0);
		}

		if (!rawMesh.uvs.empty()) {
			vbos.emplace_back(glm::value_ptr(rawMesh.uvs.front()), 2 * rawMesh.uvs.size());
			vao.addBuffer(Attributes::UV_ATTRIBUTE, vbos.back(), graphics::TypeId::Float, false, 2, 0);
		}

		if (!rawMesh.jointWeights.empty()) {
			vbos.emplace_back(rawMesh.jointWeights.data(), rawMesh.jointWeights.size());
			vao.addBuffer(Attributes::JOINT_WEIGHT_ATTRIBUTE, vbos.back(), graphics::TypeId::Float, false, 4, 0);
		}

		if (!rawMesh.jointIndices.empty()) {
			vbos.emplace_back(rawMesh.jointIndices.data(), rawMesh.jointIndices.size());
			vao.addBuffer(Attributes::JOINT_INDEX_ATTRIBUTE, vbos.back(), graphics::TypeId::Float, false, 4, 0);
		}

		graphics::IndexBuffer ibo(rawMesh.faceIndices.data(), graphics::TypeId::UnsignedShort, rawMesh.faceIndices.size());
		vao.bind();
		ibo.bind();
		vao.unbind();

		return graphics::Mesh(std::move(vbos), std::move(ibo), std::move(vao));
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
