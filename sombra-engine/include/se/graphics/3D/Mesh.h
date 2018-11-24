#ifndef MESH_H
#define MESH_H

#include <vector>
#include "../buffers/VertexBuffer.h"
#include "../buffers/IndexBuffer.h"
#include "../buffers/VertexArray.h"

namespace se::graphics {

	/**
	 * Class Mesh, it holds the buffers that creates a 3D mesh
	 */
	class Mesh
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
		 * @param	vbos the vertex buffers that coint all the vertex data of
		 *			the mesh
		 * @param	ibo the IBO of the mesh that cointains the data of the
		 *			faces of the mesh
		 * @param	vao the VAO of the mesh
		 * @note	the vertexBuffers must be already bound to the VAO with its
		 *			respective attribute indices */
		Mesh(
			std::vector<VertexBuffer>&& vbos,
			IndexBuffer&& ibo, VertexArray&& vao
		);

		/** @return the number of Indices of the faces of the Mesh */
		unsigned int getIndexCount() const;

		/** Binds the VAO of the Mesh */
		void bind() const;

		/** Unbinds the VAO of the Mesh */
		void unbind() const;
	};

}

#endif		// MESH_H
