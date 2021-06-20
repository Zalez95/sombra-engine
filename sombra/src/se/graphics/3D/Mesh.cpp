#include <algorithm>
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


	std::unique_ptr<Bindable> Mesh::clone() const
	{
		std::vector<VertexBuffer> vbos;
		for (const VertexBuffer& vbo : mVBOs) {
			vbos.emplace_back(std::move(*dynamic_cast<VertexBuffer*>(vbo.clone().get())));
		}
		IndexBuffer ibo(std::move(*dynamic_cast<IndexBuffer*>(mIBO.clone().get())));
		VertexArray vao(std::move(*dynamic_cast<VertexArray*>(mVAO.clone().get())));

		unsigned int maxAttributes = VertexArray::getMaxAttributes();
		for (unsigned int i = 0; i < maxAttributes; ++i) {
			if (mVAO.isAttributeEnabled(i)) {
				auto itVBO = std::find_if(mVBOs.begin(), mVBOs.end(), [&](const auto& vbo) {
					return mVAO.checkVertexAttributeVBOBound(i, vbo);
				});
				if (itVBO != mVBOs.end()) {
					TypeId type;
					bool normalized;
					int componentSize;
					std::size_t stride, offset;
					mVAO.getVertexAttribute(i, type, normalized, componentSize, stride, offset);

					vbos[std::distance(mVBOs.begin(), itVBO)].bind();
					vao.enableAttribute(i);
					vao.setVertexAttribute(i, type, normalized, componentSize, stride, offset);
				}
			}
		}

		vao.bind();
		ibo.bind();

		return std::make_unique<Mesh>(std::move(vbos), std::move(ibo), std::move(vao));
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
