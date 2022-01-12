#ifndef RENDERABLE_MESH_H
#define RENDERABLE_MESH_H

#include <memory>
#include "Mesh.h"
#include "Renderable3D.h"
#include "../Context.h"

namespace se::graphics {

	/**
	 * Class RenderableMesh, its a Renderable3D that holds a 3D mesh
	 */
	class RenderableMesh : public Renderable3D
	{
	private:	// Attributes
		/** The Mesh of the RenderableMesh */
		Context::TBindableRef<Mesh> mMesh;

		/** The type of primitive used for rendering @see mMesh */
		PrimitiveType mPrimitiveType;

		/** The minimum position of the RenderableMesh at each direction in
		 * world space */
		glm::vec3 mMinimum = {};

		/** The maximum position of the RenderableMesh at each direction in
		 * world space */
		glm::vec3 mMaximum = {};

		/** If @see mMinimum and @see mMaximum should be updated or not */
		bool mUpdateBounds = true;

		/** The matrix that transforms from local space to world space */
		glm::mat4 mModelMatrix = glm::mat4(1.0f);

	public:		// Functions
		/** Creates a new RenderableMesh
		 *
		 * @param	mesh a reference to the Mesh of the RenderableMesh
		 * @param	primitiveType the type of primitive used for rendering */
		RenderableMesh(
			const Context::TBindableRef<Mesh>& mesh = {},
			PrimitiveType primitiveType = PrimitiveType::Triangle
		) : mMesh(mesh), mPrimitiveType(primitiveType) {};

		/** @return	the Mesh pointed by the RenderableMesh */
		const Context::TBindableRef<Mesh>& getMesh() const { return mMesh; };

		/** Sets the Mesh pointed by the RenderableMesh
		 *
		 * @param	mesh a reference to the new Mesh pointed by the
		 *			RenderableMesh
		 * @return	a reference to the current RenderableMesh */
		RenderableMesh& setMesh(const Context::TBindableRef<Mesh>& mesh);

		/** @return	the primitive type used for rendering by the
		 *			RenderableMesh */
		PrimitiveType getPrimitiveType() const { return mPrimitiveType; };

		/** @return	the local space to world space matrix */
		const glm::mat4& getModelMatrix() const
		{ return mModelMatrix; };

		/** Sets the model matrix
		 *
		 * @param	modelMatrix local space to world space matrix
		 * @return	a reference to the current RenderableMesh object */
		RenderableMesh& setModelMatrix(const glm::mat4& modelMatrix);

		/** @copydoc Renderable3D::getBounds() */
		virtual std::pair<glm::vec3, glm::vec3> getBounds() const override
		{ return { mMinimum, mMaximum }; };

		/** @copydoc Renderable::submit(Context::Query&) */
		virtual void submit(Context::Query& q) override;

		/** Draws the current RenderableMesh (drawcall)
		 *
		 * @param	q the Context Query object used for accesing to the
		 *			Bindables */
		void draw(Context::Query& q);
	};

}

#endif		// RENDERABLE_MESH_H
