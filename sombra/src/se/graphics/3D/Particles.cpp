#include <algorithm>
#include "se/graphics/3D/Particles.h"

namespace se::graphics {

	Particles& Particles::setBuffers(
		std::vector<std::unique_ptr<VertexBuffer>>&& vbos,
		std::unique_ptr<VertexArray>&& vao
	) {
		mVBOs = std::move(vbos);
		mVAO = std::move(vao);
		return *this;
	}


	const std::vector<std::unique_ptr<VertexBuffer>>& Particles::getVBOs() const
	{
		return mVBOs;
	}


	const std::unique_ptr<VertexArray>& Particles::getVAO() const
	{
		return mVAO;
	}


	std::unique_ptr<Bindable> Particles::clone() const
	{
		std::vector<std::unique_ptr<VertexBuffer>> vbos;
		std::unique_ptr<VertexArray> vao;

		for (const auto& vbo : mVBOs) {
			vbos.push_back( std::unique_ptr<VertexBuffer>( static_cast<VertexBuffer*>(vbo->clone().release()) ) );
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
		}

		auto ret = std::make_unique<Particles>();
		ret->setBuffers(std::move(vbos), std::move(vao));
		return ret;
	}


	void Particles::bind() const
	{
		mVAO->bind();
	}


	void Particles::unbind() const
	{
		mVAO->unbind();
	}

}
