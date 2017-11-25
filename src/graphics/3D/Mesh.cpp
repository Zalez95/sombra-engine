#include "Mesh.h"
#include "../buffers/VertexBuffer.h"
#include "../buffers/IndexBuffer.h"
#include "../buffers/VertexArray.h"

namespace graphics {

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
