#include "se/graphics/3D/ParticleSystem.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::graphics {

	ParticleSystem::ParticleSystem(const ParticleSystem& other) :
		Renderable3D(other),
		mMesh(nullptr), mPrimitiveType(other.mPrimitiveType), mNumInstances(other.mNumInstances)
	{
		unsigned int maxAttributes = VertexArray::getMaxAttributes();
		for (const VertexBuffer& vbo : other.mInstanceVBOs) {
			auto bindable = vbo.clone();
			VertexBuffer vbo2 = std::move(*dynamic_cast<VertexBuffer*>(bindable.get()));

			for (unsigned int i = 0; i < maxAttributes; ++i) {
				if (other.mVAO.checkVertexAttributeVBOBound(i, vbo)) {
					mVAO.bind();
					vbo2.bind();
					mVAO.copyVertexAttribute(i, other.mVAO);
				}
			}

			mInstanceVBOs.emplace_back(std::move(vbo2));
		}

		setMesh(other.mMesh);
	}


	ParticleSystem& ParticleSystem::operator=(const ParticleSystem& other)
	{
		Renderable3D::operator=(other);

		setMesh(nullptr);
		unsigned int maxAttributes = VertexArray::getMaxAttributes();

		// Disable and unbind the VAO instance attributes and VBOs
		mVAO.bind();
		for (unsigned int i = 0; i < maxAttributes; ++i) {
			auto itVBO = std::find_if(mInstanceVBOs.begin(), mInstanceVBOs.end(), [&](const auto& vbo) {
				return mVAO.checkVertexAttributeVBOBound(i, vbo);
			});
			if (itVBO != mInstanceVBOs.end()) {
				mVAO.disableAttribute(i);
				itVBO->unbind();
			}
		}
		mInstanceVBOs.clear();

		// Add the new VAO instance attributes and VBOs
		for (const VertexBuffer& vbo : other.mInstanceVBOs) {
			auto bindable = vbo.clone();
			VertexBuffer vbo2 = std::move(*dynamic_cast<VertexBuffer*>(bindable.get()));

			for (unsigned int i = 0; i < maxAttributes; ++i) {
				if (other.mVAO.checkVertexAttributeVBOBound(i, vbo)) {
					mVAO.bind();
					vbo2.bind();
					mVAO.copyVertexAttribute(i, other.mVAO);
				}
			}

			mInstanceVBOs.emplace_back(std::move(vbo2));
		}

		mPrimitiveType = other.mPrimitiveType;
		mNumInstances = other.mNumInstances;
		setMesh(other.mMesh);

		return *this;
	}


	ParticleSystem& ParticleSystem::setMesh(MeshSPtr mesh)
	{
		unsigned int maxAttributes = VertexArray::getMaxAttributes();

		if (mMesh) {
			// Disable the Mesh vertex attributes
			const auto& vao = mMesh->getVAO();
			const auto& vbos = mMesh->getVBOs();
			for (unsigned int i = 0; i < maxAttributes; ++i) {
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
			for (unsigned int i = 0; i < maxAttributes; ++i) {
				auto itVBO = std::find_if(vbos.begin(), vbos.end(), [&](const auto& vbo) {
					return vao.checkVertexAttributeVBOBound(i, vbo);
				});
				if (itVBO != vbos.end()) {
					mVAO.bind();
					itVBO->bind();
					mVAO.copyVertexAttribute(i, vao);
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
