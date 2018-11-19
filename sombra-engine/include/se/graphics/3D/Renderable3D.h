#ifndef RENDERABLE_3D_H
#define RENDERABLE_3D_H

#include <memory>
#include <glm/glm.hpp>
#include "../PrimitiveTypes.h"

namespace se::graphics {

	class Mesh;
	class Material;
	class Texture;


	/**
	 * Class Renderable3D, it's a 3D graphic entity that holds a Mesh,
	 * its Material and its transforms
	 */
	class Renderable3D
	{
	private:	// Nested types
		typedef std::shared_ptr<Mesh> MeshSPtr;
		typedef std::shared_ptr<Material> MaterialSPtr;
		typedef std::shared_ptr<Texture> TextureSPtr;

	private:	// Attributes
		/** The Mesh of the Renderable3D */
		MeshSPtr mMesh;

		/** The Material of the Renderable3D */
		MaterialSPtr mMaterial;

		/** The Texture of the Renderable3D */
		TextureSPtr mTexture;

		/** The render flags of the materials */
		unsigned char mRenderFlags;

		/** The matrix that transforms the coordinates of the Mesh from
		 * Local space to World space */
		glm::mat4 mModelMatrix;

	public:		// Functions
		/** Creates a new Renderable3D
		 *
		 * @param	mesh a pointer to the Mesh of the Renderable3D
		 * @param	material a pointer to the Material of the Renderable3D
		 * @param	texture a pointer to the Texture of the Renderable3D
		 * @param	renderFlags the render flags of the Renderable3D */
		Renderable3D(
			MeshSPtr mesh, MaterialSPtr material, TextureSPtr texture,
			unsigned char renderFlags = RenderFlags::DEFAULT
		) : mMesh(mesh), mMaterial(material), mTexture(texture),
			mRenderFlags(renderFlags), mModelMatrix(1.0f) {};

		/** Class destructor */
		~Renderable3D() {};

		/** @return a pointer to the Mesh of the Renderable3D */
		inline const MeshSPtr getMesh() const { return mMesh; };

		/** @return a pointer to the Material of the Renderable3D */
		inline const MaterialSPtr getMaterial() const { return mMaterial; };

		/** @return a pointer to the Texture of the Renderable3D */
		inline const TextureSPtr getTexture() const { return mTexture; };

		/** @return the model matrix of the Renderable3D */
		inline glm::mat4 getModelMatrix() const { return mModelMatrix; };

		/** Sets the Model Matrix of the Renderable3D
		 *
		 * @param	modelMatrix the new model matrix of the Renderable3D */
		inline void setModelMatrix(const glm::mat4& modelMatrix)
		{ mModelMatrix = modelMatrix; };

		/** @return the RenderFlags of the Renderable3D */
		inline unsigned char getRenderFlags() const { return mRenderFlags; };
	};

}

#endif		// RENDERABLE_3D_H
