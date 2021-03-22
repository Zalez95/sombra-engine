#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glm/glm.hpp>
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

		/** The minimum position of the Mesh at each direction */
		glm::vec3 mMinimum;

		/** The maximum position of the Mesh at each direction */
		glm::vec3 mMaximum;

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

		/** Sets the bounds of the Mesh
		 *
		 * @param	minimum the minimum position of the mesh at each direction
		 * @param	minimum the maximum position of the mesh at each
		 *			direction */
		void setBounds(const glm::vec3& minimum, const glm::vec3& maximum);

		/** @return	the minimum and maximum position of the Mesh in each
		 *			direction */
		std::pair<glm::vec3, glm::vec3> getBounds() const;

		/** @return a reference to the VertexBuffers of the Mesh */
		const std::vector<VertexBuffer>& getVBOs() const;

		/** @return a reference to the IndexBuffer of the Mesh */
		const IndexBuffer& getIBO() const;

		/** @return a reference to the VertexArray of the Mesh */
		const VertexArray& getVAO() const;

		/** Binds the VAO of the Mesh */
		virtual void bind() const override;

		/** Unbinds the VAO of the Mesh */
		virtual void unbind() const override;
	};

}

#endif		// MESH_H
