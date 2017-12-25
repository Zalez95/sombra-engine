#include "fe/graphics/3D/Mesh.h"
#include "fe/graphics/buffers/VertexBuffer.h"
#include "fe/graphics/buffers/IndexBuffer.h"
#include "fe/graphics/buffers/VertexArray.h"

namespace fe { namespace graphics {

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

}}
