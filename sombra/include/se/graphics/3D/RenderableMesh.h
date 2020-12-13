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

	public:		// Functions
		/** Creates a new RenderableMesh
		 *
		 * @param	mesh a pointer to the Mesh of the RenderableMesh */
		RenderableMesh(MeshSPtr mesh);

		/** @copydoc Renderable3D::setModelMatrix(const glm::mat4&) */
		virtual Renderable3D& setModelMatrix(
			const glm::mat4& modelMatrix
		) override;

		/** @copydoc Renderable3D::draw() */
		virtual void draw() override;
	};

}

#endif		// RENDERABLE_MESH_H
