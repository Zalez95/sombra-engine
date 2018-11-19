#include "se/graphics/buffers/VertexBuffer.h"
#include "se/graphics/buffers/IndexBuffer.h"
#include "se/graphics/buffers/VertexArray.h"
#include "se/graphics/3D/Mesh.h"

namespace se::graphics {

	Mesh::Mesh(
		const std::string& name,
		std::vector<VertexBufferUPtr>&& vbos,
		IndexBufferUPtr ibo, VertexArrayUPtr vao
	) : mName(name),
		mVBOs(std::move(vbos)), mIBO(std::move(ibo)), mVAO(std::move(vao)) {}


	Mesh::~Mesh() {}


	unsigned int Mesh::getIndexCount() const
	{
		return mIBO->getIndexCount();
	}


	void Mesh::bind() const
	{
		mVAO->bind();
	}


	void Mesh::unbind() const
	{
		mVAO->unbind();
	}

}
