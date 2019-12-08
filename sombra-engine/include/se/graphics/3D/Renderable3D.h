#ifndef RENDERABLE_3D_H
#define RENDERABLE_3D_H

#include <memory>
#include <vector>
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
		using MeshSPtr = std::shared_ptr<Mesh>;
		using MaterialSPtr = std::shared_ptr<Material>;

	private:	// Attributes
		/** The Mesh of the Renderable3D */
		MeshSPtr mMesh;

		/** The Material of the Renderable3D */
		MaterialSPtr mMaterial;

		/** The matrix that transforms the coordinates of the Mesh from
		 * Local space to World space */
		glm::mat4 mModelMatrix;

		/** If the Renderable3D has Skeleton or not */
		bool mHasSkeleton;

		/** The skeleton joints matrices in local space.
		 * @note	this matrices must have applied the inverse bind matrices
		 *			of the joints */
		std::vector<glm::mat4> mJointMatrices;

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
		) : mMesh(mesh), mMaterial(material),
			mModelMatrix(modelMatrix), mHasSkeleton(false) {};

		/** @return	a pointer to the Mesh of the Renderable3D */
		const MeshSPtr getMesh() const { return mMesh; };

		/** @return	a pointer to the Material of the Renderable3D */
		const MaterialSPtr getMaterial() const { return mMaterial; };

		/** @return	the model matrix of the Renderable3D */
		glm::mat4 getModelMatrix() const { return mModelMatrix; };

		/** @return	true if the Renderable3D has an skeletal animation, false
		 *			otherwise */
		bool hasSkeleton() const { return mHasSkeleton; };

		/** @return	the joint matrices of the Renderable3D */
		const std::vector<glm::mat4>& getJointMatrices() const
		{ return mJointMatrices; };

		/** Sets the Model Matrix of the Renderable3D
		 *
		 * @param	modelMatrix the new model matrix of the Renderable3D */
		void setModelMatrix(const glm::mat4& modelMatrix)
		{ mModelMatrix = modelMatrix; };

		/** Sets the joint transforms matrices of the Renderable3D
		 *
		 * @param	jointMatrices a vector with the joint matrices of the
		 *			Renderable3D in local space */
		void setJointMatrices(const std::vector<glm::mat4>& jointMatrices)
		{ mJointMatrices = jointMatrices; mHasSkeleton = true; };
	};

}

#endif		// RENDERABLE_3D_H
