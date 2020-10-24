#include "se/graphics/3D/RenderableMesh.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::graphics {

	void RenderableMesh::draw()
	{
		mMesh->bind();
		GraphicsOperations::drawIndexed(
			PrimitiveType::Triangle,
			mMesh->getIBO().getIndexCount(), mMesh->getIBO().getIndexType()
		);
	}

}
