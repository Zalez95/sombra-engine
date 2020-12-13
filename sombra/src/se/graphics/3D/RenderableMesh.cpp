#include <array>
#include "se/graphics/3D/RenderableMesh.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::graphics {

	RenderableMesh::RenderableMesh(MeshSPtr mesh) : mMesh(mesh)
	{
		if (mMesh) {
			std::tie(mMinimum, mMaximum) = mMesh->getBounds();
		}
	}


	Renderable3D& RenderableMesh::setModelMatrix(const glm::mat4& modelMatrix)
	{
		Renderable3D::setModelMatrix(modelMatrix);

		if (mMesh) {
			auto [localMin, localMax] = mMesh->getBounds();

			std::array<glm::vec3, 8> localAABBVertices = {
				glm::vec3(localMin.x, localMin.y, localMin.z),
				glm::vec3(localMin.x, localMin.y, localMax.z),
				glm::vec3(localMin.x, localMax.y, localMin.z),
				glm::vec3(localMin.x, localMax.y, localMax.z),
				glm::vec3(localMax.x, localMin.y, localMin.z),
				glm::vec3(localMax.x, localMin.y, localMax.z),
				glm::vec3(localMax.x, localMax.y, localMin.z),
				glm::vec3(localMax.x, localMax.y, localMax.z)
			};

			mMinimum = mMaximum = mModelMatrix * glm::vec4(localAABBVertices[0], 1.0f);
			for (std::size_t i = 1; i < 8; ++i) {
				glm::vec3 vw = mModelMatrix * glm::vec4(localAABBVertices[i], 1.0f);
				mMinimum = glm::min(mMinimum, vw);
				mMaximum = glm::max(mMaximum, vw);
			}
		}

		return *this;
	}


	void RenderableMesh::draw()
	{
		mMesh->bind();
		GraphicsOperations::drawIndexed(
			PrimitiveType::Triangle,
			mMesh->getIBO().getIndexCount(), mMesh->getIBO().getIndexType()
		);
	}

}
