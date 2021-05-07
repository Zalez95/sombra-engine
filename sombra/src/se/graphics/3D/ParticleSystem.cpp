#include "se/graphics/3D/ParticleSystem.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::graphics {

	ParticleSystem& ParticleSystem::setMesh(MeshSPtr mesh)
	{
		if (mMesh) {
			// Disable the Mesh vertex attributes
			const auto& vao = mMesh->getVAO();
			const auto& vbos = mMesh->getVBOs();
			for (unsigned int i = 0; i < VertexArray::getMaxAttributes(); ++i) {
				if (vao.isAttributeEnabled(i)) {
					mVAO.disableAttribute(i);
				}

				auto itVBO = std::find_if(vbos.begin(), vbos.end(), [&](const auto& vbo) {
					return vao.checkVertexAttributeVBOBound(i, vbo);
				});
				if (itVBO != vbos.end()) {
					mVAO.bind();
					itVBO->unbind();
				}
			}

			mVAO.bind();
			mMesh->getIBO().unbind();
		}

		mMesh = mesh;

		if (mMesh) {
			// Enable the Mesh vertex attributes
			const auto& vao = mMesh->getVAO();
			const auto& vbos = mMesh->getVBOs();
			for (unsigned int i = 0; i < VertexArray::getMaxAttributes(); ++i) {
				auto itVBO = std::find_if(vbos.begin(), vbos.end(), [&](const auto& vbo) {
					return vao.checkVertexAttributeVBOBound(i, vbo);
				});
				if (itVBO != vbos.end()) {
					mVAO.bind();
					itVBO->bind();
				}

				if (vao.isAttributeEnabled(i)) {
					bool integerAttribute = vao.isIntegerAttribute(i);
					bool doubleAttribute = vao.isDoubleAttribute(i);

					TypeId type;
					bool normalized;
					int componentSize;
					std::size_t stride, offset;
					vao.getVertexAttribute(i, type, normalized, componentSize, stride, offset);

					mVAO.bind();
					mVAO.enableAttribute(i);
					if (integerAttribute) {
						mVAO.setVertexIntegerAttribute(i, type, componentSize, stride, offset);
					}
					else if (doubleAttribute) {
						mVAO.setVertexDoubleAttribute(i, type, componentSize, stride, offset);
					}
					else {
						mVAO.setVertexAttribute(i, type, normalized, componentSize, stride, offset);
					}
				}
			}

			mVAO.bind();
			mMesh->getIBO().bind();
		}

		return *this;
	}


	void ParticleSystem::setBounds(const glm::vec3& minimum, const glm::vec3& maximum)
	{
		mMinimum = minimum;
		mMaximum = maximum;
	}


	void ParticleSystem::drawInstances()
	{
		mVAO.bind();
		GraphicsOperations::drawIndexedInstanced(
			mPrimitiveType,
			mMesh->getIBO().getIndexCount(), mMesh->getIBO().getIndexType(),
			0, mNumInstances
		);
	}

}
