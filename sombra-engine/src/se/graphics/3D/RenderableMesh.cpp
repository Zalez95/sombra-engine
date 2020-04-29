#include "se/graphics/3D/RenderableMesh.h"
#include "se/graphics/3D/Mesh.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::graphics {

	RenderableMesh::RenderableMesh(MeshSPtr mesh) : mMesh(mesh)
	{
		addBindable(mMesh);
	}


	void RenderableMesh::draw()
	{
		GraphicsOperations::drawIndexed(
			PrimitiveType::Triangle,
			mMesh->getIBO().getIndexCount(), mMesh->getIBO().getIndexType()
		);
	}

}
