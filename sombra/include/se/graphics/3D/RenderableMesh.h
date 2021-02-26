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
		/** The Mesh of the RenderableMesh */
		MeshSPtr mMesh;

		/** The type of primitive used for rendering @see mMesh */
		PrimitiveType mPrimitiveType;

	public:		// Functions
		/** Creates a new RenderableMesh
		 *
		 * @param	mesh a pointer to the Mesh of the RenderableMesh
		 * @param	primitiveMesh the type of primitive used for rendering */
		RenderableMesh(
			MeshSPtr mesh,
			PrimitiveType primitiveType = PrimitiveType::Triangle
		);

		/** @return	the Mesh pointed by the RenderableMesh */
		MeshSPtr getMesh() const { return mMesh; };

		/** Sets the Mesh pointed by the RenderableMesh
		 *
		 * @param	mesh a pointer to the new Mesh pointed by the
		 *			RenderableMesh
		 * @return	a reference to the current RenderableMesh */
		RenderableMesh& setMesh(MeshSPtr mesh);

		/** @copydoc Renderable3D::setModelMatrix(const glm::mat4&) */
		virtual Renderable3D& setModelMatrix(
			const glm::mat4& modelMatrix
		) override;

		/** @copydoc Renderable3D::draw() */
		virtual void draw() override;
	};

}

#endif		// RENDERABLE_MESH_H
