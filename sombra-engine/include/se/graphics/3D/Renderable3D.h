#ifndef RENDERABLE_3D_H
#define RENDERABLE_3D_H

#include <memory>
#include <glm/glm.hpp>

namespace se::graphics {

	class Mesh;
	struct Material;


	/**
	 * Class Renderable3D, it's a 3D graphic entity that holds a Mesh,
	 * its Material and its transforms
	 */
	class Renderable3D
	{
	private:	// Nested types
		typedef std::shared_ptr<Mesh> MeshSPtr;
		typedef std::shared_ptr<Material> MaterialSPtr;

	private:	// Attributes
		/** The Mesh of the Renderable3D */
		MeshSPtr mMesh;

		/** The Material of the Renderable3D */
		MaterialSPtr mMaterial;

		/** The matrix that transforms the coordinates of the Mesh from
		 * Local space to World space */
		glm::mat4 mModelMatrix;

	public:		// Functions
		/** Creates a new Renderable3D
		 *
		 * @param	mesh a pointer to the Mesh of the Renderable3D
		 * @param	material a pointer to the Material of the Renderable3D
		 * @param	modelMatrix the model matrix of the Renderable3D, by default
		 *			the identity matrix */
		Renderable3D(
			MeshSPtr mesh, MaterialSPtr material,
			const glm::mat4& modelMatrix = glm::mat4(1.0f)
		) : mMesh(mesh), mMaterial(material), mModelMatrix(modelMatrix) {};

		/** @return a pointer to the Mesh of the Renderable3D */
		inline const MeshSPtr getMesh() const { return mMesh; };

		/** @return a pointer to the Material of the Renderable3D */
		inline const MaterialSPtr getMaterial() const { return mMaterial; };

		/** @return the model matrix of the Renderable3D */
		inline glm::mat4 getModelMatrix() const { return mModelMatrix; };

		/** Sets the Model Matrix of the Renderable3D
		 *
		 * @param	modelMatrix the new model matrix of the Renderable3D */
		inline void setModelMatrix(const glm::mat4& modelMatrix)
		{ mModelMatrix = modelMatrix; };
	};

}

#endif		// RENDERABLE_3D_H
