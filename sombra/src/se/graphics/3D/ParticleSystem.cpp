#include "se/graphics/3D/ParticleSystem.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::graphics {

	ParticleSystem::ParticleSystem(
		const Context::TBindableRef<Mesh>& mesh,
		const Context::TBindableRef<Particles>& particles,
		PrimitiveType primitiveType
	) : mMesh(mesh), mParticles(particles), mPrimitiveType(primitiveType)
	{
		mUpdateVAO = true;
	}


	ParticleSystem::ParticleSystem(const ParticleSystem& other) :
		Renderable3D(other), mMesh(other.mMesh), mParticles(other.mParticles),
		mPrimitiveType(other.mPrimitiveType), mNumInstances(other.mNumInstances),
		mMinimum(other.mMinimum), mMaximum(other.mMaximum)
	{
		mUpdateVAO = true;
	}


	ParticleSystem& ParticleSystem::operator=(const ParticleSystem& other)
	{
		Renderable3D::operator=(other);

		mMesh = other.mMesh;
		mParticles = other.mParticles;
		mPrimitiveType = other.mPrimitiveType;
		mNumInstances = other.mNumInstances;
		mMinimum = other.mMinimum;
		mMaximum = other.mMaximum;
		mUpdateVAO = true;

		return *this;
	}


	ParticleSystem& ParticleSystem::setMesh(const Context::TBindableRef<Mesh>& mesh)
	{
		mMesh = mesh;
		mUpdateVAO = true;
		return *this;
	}


	ParticleSystem& ParticleSystem::setParticles(const Context::TBindableRef<Particles>& particles)
	{
		mParticles = particles;
		mUpdateVAO = true;
		return *this;
	}


	void ParticleSystem::setBounds(const glm::vec3& minimum, const glm::vec3& maximum)
	{
		mMinimum = minimum;
		mMaximum = maximum;
	}


	void ParticleSystem::submit(Context::Query& q)
	{
		if (mUpdateVAO) {
			mUpdateVAO = false;

			unsigned int maxAttributes = VertexArray::getMaxAttributes();
			mVAO = std::make_unique<VertexArray>();

			if (mMesh) {
				// Enable the Mesh vertex attributes
				const auto& vao = q.getTBindable(mMesh)->getVAO();
				const auto& vbos = q.getTBindable(mMesh)->getVBOs();
				for (unsigned int i = 0; i < maxAttributes; ++i) {
					auto itVBO = std::find_if(vbos.begin(), vbos.end(), [&](const auto& vbo) {
						return vao->checkVertexAttributeVBOBound(i, *vbo);
					});
					if (itVBO != vbos.end()) {
						mVAO->bind();
						(*itVBO)->bind();
						mVAO->copyVertexAttribute(i, *vao);
					}
				}

				mVAO->bind();
				q.getTBindable(mMesh)->getIBO()->bind();
			}

			if (mParticles) {
				// Enable the Particles vertex attributes
				const auto& vao = q.getTBindable(mParticles)->getVAO();
				const auto& vbos = q.getTBindable(mParticles)->getVBOs();
				for (unsigned int i = 0; i < maxAttributes; ++i) {
					auto itVBO = std::find_if(vbos.begin(), vbos.end(), [&](const auto& vbo) {
						return vao->checkVertexAttributeVBOBound(i, *vbo);
					});
					if (itVBO != vbos.end()) {
						mVAO->bind();
						(*itVBO)->bind();
						mVAO->copyVertexAttribute(i, *vao);
					}
				}
			}
		}

		Renderable::submit(q);
	}


	void ParticleSystem::drawInstances(Context::Query& q)
	{
		mVAO->bind();
		GraphicsOperations::drawIndexedInstanced(
			mPrimitiveType,
			q.getTBindable(mMesh)->getIBO()->getIndexCount(),
			q.getTBindable(mMesh)->getIBO()->getIndexType(),
			0, mNumInstances
		);
	}

}
