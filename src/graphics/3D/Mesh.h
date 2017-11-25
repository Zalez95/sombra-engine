#ifndef MESH_H
#define MESH_H

#include <memory>
#include <vector>
#include <string>

namespace graphics {

	class VertexBuffer;
	class IndexBuffer;
	class VertexArray;


	/**
	 * Class Mesh, it holds the buffers that creates a 3D mesh
	 * TODO
	 */
	class Mesh
	{
	private:	// Nested Types
		typedef std::unique_ptr<VertexBuffer> VertexBufferUPtr;
		typedef std::unique_ptr<IndexBuffer> IndexBufferUPtr;
		typedef std::unique_ptr<VertexArray> VertexArrayUPtr;

	private:	// Attributes
		/** The name of the Mesh */
		const std::string mName;

		/** The vertex buffers of the Mesh */
		std::vector<VertexBufferUPtr> mVBOs;

		/** The IBO of the Mesh */
		IndexBufferUPtr mIBO;

		/** The VAO of the Mesh */
		VertexArrayUPtr mVAO;

	public:		// Functions
		/** Creates a new Mesh from the given data
		 *
		 * @param	name the name of the Mesh
		 * @param	vbos the vertex buffers that coint all the vertex data of
		 *			the mesh
		 * @param	ibo the IBO of the mesh that cointains the data of the
		 *			faces of the mesh
		 * @param	vao the VAO of the mesh
		 * @note	the vertexBuffers must be already bound to the VAO with its
		 *			respective attribute indices */
		Mesh(
			const std::string& name,
			std::vector<VertexBufferUPtr>&& vbos,
			IndexBufferUPtr ibo, VertexArrayUPtr vao
		);

		/** Class destructor */
		~Mesh();

		/** @return the name of the Mesh */
		inline std::string getName() const { return mName; };

		/** @return the number of Indices of the faces of the Mesh */
		unsigned int getIndexCount() const;

		/** Binds the VAO of the Mesh */
		void bind() const;

		/** Unbinds the VAO of the Mesh */
		void unbind() const;
	};

}

#endif		// MESH_H
