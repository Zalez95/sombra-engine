#ifndef RENDERABLE_MESH_H
#define RENDERABLE_MESH_H

#include <memory>
#include "Mesh.h"
#include "Renderable3D.h"

namespace se::graphics {

	/**
	 * Class RenderableMesh, its a Renderable3D that holds a 3D mesh
	 */
	class RenderableMesh : public Renderable3D
	{
	private:	// Nested types
		using MeshSPtr = std::shared_ptr<Mesh>;

	private:	// Attributes
		/** The matrix that transforms from local space to world space */
		glm::mat4 mModelMatrix = glm::mat4(1.0f);

		/** The Mesh of the RenderableMesh */
		MeshSPtr mMesh;

		/** The type of primitive used for rendering @see mMesh */
		PrimitiveType mPrimitiveType;

	public:		// Functions
		/** Creates a new RenderableMesh
		 *
		 * @param	mesh a pointer to the Mesh of the RenderableMesh
		 * @param	primitiveType the type of primitive used for rendering */
		RenderableMesh(
			MeshSPtr mesh = nullptr,
			PrimitiveType primitiveType = PrimitiveType::Triangle
		);

		/** @return	the Mesh pointed by the RenderableMesh */
		MeshSPtr getMesh() const { return mMesh; };

		/** @return	the primitive type used for rendering by the
		 *			RenderableMesh */
		PrimitiveType getPrimitiveType() const { return mPrimitiveType; };

		/** Sets the model matrix
		 *
		 * @param	modelMatrix local space to world space matrix
		 * @return	a reference to the current RenderableMesh object */
		RenderableMesh& setModelMatrix(const glm::mat4& modelMatrix);

		/** @return	the local space to world space matrix */
		const glm::mat4& getModelMatrix() const
		{ return mModelMatrix; };

		/** Sets the Mesh pointed by the RenderableMesh
		 *
		 * @param	mesh a pointer to the new Mesh pointed by the
		 *			RenderableMesh
		 * @return	a reference to the current RenderableMesh */
		RenderableMesh& setMesh(MeshSPtr mesh);

		/** Draws the current RenderableMesh (drawcall) */
		void draw();
	};

}

#endif		// RENDERABLE_MESH_H
