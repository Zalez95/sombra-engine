#ifndef MESH_H
#define MESH_H

#include <vector>
#include "../core/VertexBuffer.h"
#include "../core/IndexBuffer.h"
#include "../core/VertexArray.h"

namespace se::graphics {

	/**
	 * Class Mesh, it holds the buffers that creates a 3D mesh
	 */
	class Mesh : public Bindable
	{
	private:	// Attributes
		/** The vertex buffers of the Mesh */
		std::vector<VertexBuffer> mVBOs;

		/** The IBO of the Mesh */
		IndexBuffer mIBO;

		/** The VAO of the Mesh */
		VertexArray mVAO;

	public:		// Functions
		/** Creates a new Mesh from the given data
		 *
		 * @param	vbos the vertex buffers that cointain all the vertex data of
		 *			the mesh
		 * @param	ibo the IBO of the mesh that cointains the data of the
		 *			faces of the mesh
		 * @param	vao the VAO of the mesh
		 * @note	the IBO and the VBOs must be already bound to the VAO with
		 *			its respective attribute indices */
		Mesh(
			std::vector<VertexBuffer>&& vbos,
			IndexBuffer&& ibo, VertexArray&& vao
		);

		/** @return a reference to the IndexBuffer of the Mesh */
		const IndexBuffer& getIBO() const;

		/** Binds the VAO of the Mesh */
		virtual void bind() const override;

		/** Unbinds the VAO of the Mesh */
		virtual void unbind() const override;
	};

}

#endif		// MESH_H
