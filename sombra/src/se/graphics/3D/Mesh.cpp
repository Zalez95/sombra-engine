#include <algorithm>
#include "se/graphics/3D/Mesh.h"

namespace se::graphics {

	Mesh& Mesh::setBuffers(
		std::vector<std::unique_ptr<VertexBuffer>>&& vbos,
		std::unique_ptr<IndexBuffer>&& ibo, std::unique_ptr<VertexArray>&& vao
	) {
		mVBOs = std::move(vbos);
		mIBO = std::move(ibo);
		mVAO = std::move(vao);
		return *this;
	}


	Mesh& Mesh::setBounds(const glm::vec3& minimum, const glm::vec3& maximum)
	{
		mMinimum = minimum;
		mMaximum = maximum;
		return *this;
	}


	std::pair<glm::vec3, glm::vec3> Mesh::getBounds() const
	{
		return { mMinimum, mMaximum };
	}


	const std::vector<std::unique_ptr<VertexBuffer>>& Mesh::getVBOs() const
	{
		return mVBOs;
	}


	const std::unique_ptr<IndexBuffer>& Mesh::getIBO() const
	{
		return mIBO;
	}


	const std::unique_ptr<VertexArray>& Mesh::getVAO() const
	{
		return mVAO;
	}


	std::unique_ptr<Bindable> Mesh::clone() const
	{
		std::vector<std::unique_ptr<VertexBuffer>> vbos;
		std::unique_ptr<IndexBuffer> ibo;
		std::unique_ptr<VertexArray> vao;

		for (const auto& vbo : mVBOs) {
			vbos.push_back( std::unique_ptr<VertexBuffer>( static_cast<VertexBuffer*>(vbo->clone().release())) );
		}

		if (mIBO) {
			ibo = std::unique_ptr<IndexBuffer>( static_cast<IndexBuffer*>(mIBO->clone().release()) );
		}

		if (mVAO) {
			vao = std::make_unique<VertexArray>();

			unsigned int maxAttributes = VertexArray::getMaxAttributes();
			for (unsigned int i = 0; i < maxAttributes; ++i) {
				if (mVAO->isAttributeEnabled(i)) {
					auto itVBO = std::find_if(mVBOs.begin(), mVBOs.end(), [&](const auto& vbo) {
						return mVAO->checkVertexAttributeVBOBound(i, *vbo);
					});
					if (itVBO != mVBOs.end()) {
						vao->bind();
						vbos[std::distance(mVBOs.begin(), itVBO)]->bind();
						vao->copyVertexAttribute(i, *mVAO);
					}
				}
			}

			if (ibo) {
				vao->bind();
				ibo->bind();
			}
		}

		auto ret = std::make_unique<Mesh>();
		ret->setBuffers(std::move(vbos), std::move(ibo), std::move(vao));
		ret->setBounds(mMinimum, mMaximum);
		return ret;
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
