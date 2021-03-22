#include <array>
#include "se/graphics/3D/RenderableMesh.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/utils/MathUtils.h"

namespace se::graphics {

	RenderableMesh::RenderableMesh(MeshSPtr mesh, PrimitiveType primitiveType) :
		mMesh(mesh), mPrimitiveType(primitiveType)
	{
		if (mMesh) {
			std::tie(mMinimum, mMaximum) = mMesh->getBounds();
		}
	}


	RenderableMesh& RenderableMesh::setMesh(MeshSPtr mesh)
	{
		mMesh = mesh;
		setModelMatrix(getModelMatrix());	// force to recalculate the AABB
		return *this;
	}


	Renderable3D& RenderableMesh::setModelMatrix(const glm::mat4& modelMatrix)
	{
		Renderable3D::setModelMatrix(modelMatrix);

		if (mMesh) {
			auto [localMin, localMax] = mMesh->getBounds();
			std::tie(mMinimum, mMaximum) = utils::getBoundsWorld(localMin, localMax, mModelMatrix);
		}

		return *this;
	}


	void RenderableMesh::draw()
	{
		mMesh->bind();
		GraphicsOperations::drawIndexed(
			mPrimitiveType,
			mMesh->getIBO().getIndexCount(), mMesh->getIBO().getIndexType()
		);
	}

}
