#include "se/graphics/3D/Mesh.h"

namespace se::graphics {

	Mesh::Mesh(std::vector<VertexBuffer>&& vbos, IndexBuffer&& ibo, VertexArray&& vao) :
		mVBOs(std::move(vbos)), mIBO(std::move(ibo)), mVAO(std::move(vao)),
		mMinimum(0.0f), mMaximum(0.0f) {}


	void Mesh::setBounds(const glm::vec3& minimum, const glm::vec3& maximum)
	{
		mMinimum = minimum;
		mMaximum = maximum;
	}


	std::pair<glm::vec3, glm::vec3> Mesh::getBounds() const
	{
		return { mMinimum, mMaximum };
	}


	const std::vector<VertexBuffer>& Mesh::getVBOs() const
	{
		return mVBOs;
	}


	const IndexBuffer& Mesh::getIBO() const
	{
		return mIBO;
	}


	const VertexArray& Mesh::getVAO() const
	{
		return mVAO;
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
