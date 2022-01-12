#include <array>
#include "se/graphics/3D/Mesh.h"
#include "se/graphics/3D/RenderableMesh.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/utils/MathUtils.h"

namespace se::graphics {

	RenderableMesh& RenderableMesh::setMesh(const Context::TBindableRef<Mesh>& mesh)
	{
		mMesh = mesh;
		mUpdateBounds = true;
		return *this;
	}


	RenderableMesh& RenderableMesh::setModelMatrix(const glm::mat4& modelMatrix)
	{
		mModelMatrix = modelMatrix;
		mUpdateBounds = true;
		return *this;
	}


	void RenderableMesh::submit(Context::Query& q)
	{
		if (mUpdateBounds) {
			mUpdateBounds = false;

			if (mMesh) {
				auto [localMin, localMax] = q.getTBindable(mMesh)->getBounds();
				std::tie(mMinimum, mMaximum) = utils::getBoundsWorld(localMin, localMax, mModelMatrix);
			}
			else {
				mMinimum = mMaximum = {};
			}
		}

		Renderable::submit(q);
	}


	void RenderableMesh::draw(Context::Query& q)
	{
		q.getBindable(mMesh)->bind();
		GraphicsOperations::drawIndexed(
			mPrimitiveType,
			q.getTBindable(mMesh)->getIBO()->getIndexCount(),
			q.getTBindable(mMesh)->getIBO()->getIndexType()
		);
	}

}
