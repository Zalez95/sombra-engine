#ifndef MESH_H
#define MESH_H

#include <memory>
#include <vector>
#include <string>
#include "AABB.h"

namespace graphics {

	class VertexBuffer;
	class IndexBuffer;
	class VertexArray;


	/**
	 * Class Mesh, it holds the vertex, indices and the weights of its
	 * bones. Also it holds the indices to its buffers
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

		/** The bounds of the Mesh in global space stored as an AABB */
		AABB mBounds;

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
			std::vector<VertexBufferUPtr> vbos,
			IndexBufferUPtr ibo, VertexArrayUPtr vao
		);

		/** Class destructor */
		~Mesh();

		/** @return the name of the Mesh */
		inline std::string getName() const { return mName; };

		/** @return the number of Indices of the faces of the Mesh */
		unsigned int getIndexCount() const;

		/** @return a struct AABB with the maximum and minimum coordinates in
		 * Global Space of the vertices of the mesh in each axis */
		inline AABB getBounds() const { return mBounds; };

		/** Sets the bounds of the Mesh
		 *
		 * @param	bounds the new bounds of the Mesh stored as an AABB */
		inline void setBounds(const AABB& bounds) { mBounds = bounds; };

		/** Binds the VAO of the Mesh */
		void bindVAO() const;
	};

}

#endif		// MESH_H
