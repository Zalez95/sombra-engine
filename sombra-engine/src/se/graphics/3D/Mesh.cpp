#include "se/graphics/3D/Mesh.h"

namespace se::graphics {

	Mesh::Mesh(
		std::vector<VertexBuffer>&& vbos,
		IndexBuffer&& ibo, VertexArray&& vao
	) : mVBOs(std::move(vbos)), mIBO(std::move(ibo)), mVAO(std::move(vao)) {}


	unsigned int Mesh::getIndexCount() const
	{
		return mIBO.getIndexCount();
	}


	void Mesh::bind() const
	{
		mVAO.bind();
	}


	void Mesh::unbind() const
	{
		mVAO.unbind();
	}

}