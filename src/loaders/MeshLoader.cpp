#include "MeshLoader.h"
#include "../graphics/3D/Mesh.h"
#include "../graphics/buffers/VertexBuffer.h"
#include "../graphics/buffers/IndexBuffer.h"
#include "../graphics/buffers/VertexArray.h"

namespace loaders {

	MeshLoader::MeshUPtr MeshLoader::createMesh(
		const std::string& name,
		const std::vector<GLfloat>& positions,
		const std::vector<GLfloat>& normals,
		const std::vector<GLfloat>& uvs,
		const std::vector<GLushort>& faceIndices
	) const {
		auto vao = std::make_unique<graphics::VertexArray>();
		auto ibo = std::make_unique<graphics::IndexBuffer>(faceIndices.data(), faceIndices.size());
		std::vector<std::unique_ptr<graphics::VertexBuffer>> vbos;
		std::unique_ptr<graphics::VertexBuffer> tmp;

		tmp = std::make_unique<graphics::VertexBuffer>(positions.data(), positions.size(), 3);
		vao->addBuffer(tmp.get(), POSITION_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<graphics::VertexBuffer>(normals.data(), normals.size(), 3);
		vao->addBuffer(tmp.get(), NORMAL_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<graphics::VertexBuffer>(uvs.data(), uvs.size(), 2);
		vao->addBuffer(tmp.get(), UV_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		vao->bind();
		ibo->bind();
		vao->unbind();

		return std::make_unique<graphics::Mesh>(name, std::move(vbos), std::move(ibo), std::move(vao));
	}


	MeshLoader::MeshUPtr MeshLoader::createMesh(
		const std::string& name,
		const std::vector<GLfloat>& positions,
		const std::vector<GLfloat>& normals,
		const std::vector<GLfloat>& uvs,
		const std::vector<GLfloat>& jointWeights,
		const std::vector<GLushort>& jointIndices,
		const std::vector<GLushort>& faceIndices
	) const {
		std::vector<std::unique_ptr<graphics::VertexBuffer>> vbos;
		auto ibo = std::make_unique<graphics::IndexBuffer>(faceIndices.data(), faceIndices.size());
		auto vao = std::make_unique<graphics::VertexArray>();

		std::unique_ptr<graphics::VertexBuffer> tmp;
		tmp = std::make_unique<graphics::VertexBuffer>(positions.data(), positions.size(), 3);
		vao->addBuffer(tmp.get(), POSITION_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<graphics::VertexBuffer>(normals.data(), normals.size(), 3);
		vao->addBuffer(tmp.get(), NORMAL_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<graphics::VertexBuffer>(uvs.data(), uvs.size(), 2);
		vao->addBuffer(tmp.get(), UV_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<graphics::VertexBuffer>(jointWeights.data(), jointWeights.size(), 4);
		vao->addBuffer(tmp.get(), JOINT_WEIGHT_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		tmp = std::make_unique<graphics::VertexBuffer>(jointIndices.data(), jointIndices.size(), 4);
		vao->addBuffer(tmp.get(), JOINT_INDEX_ATTRIBUTE);
		vbos.push_back(std::move(tmp));

		vao->bind();
		ibo->bind();
		vao->unbind();

		return std::make_unique<graphics::Mesh>(name, std::move(vbos), std::move(ibo), std::move(vao));
	}

}
