#ifndef MESH_H
#define MESH_H

#include <vector>
#include <memory>
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
		std::vector<std::unique_ptr<VertexBuffer>> mVBOs;

		/** The IBO of the Mesh */
		std::unique_ptr<IndexBuffer> mIBO;

		/** The VAO of the Mesh */
		std::unique_ptr<VertexArray> mVAO;

		/** The minimum position of the Mesh at each direction */
		glm::vec3 mMinimum = glm::vec3(0.0f);

		/** The maximum position of the Mesh at each direction */
		glm::vec3 mMaximum = glm::vec3(0.0f);

	public:		// Functions
		/** Sets the buffers of the Mesh
		 *
		 * @param	vbos the vertex buffers that cointain all the vertex data of
		 *			the mesh
		 * @param	ibo the IBO of the mesh that cointains the data of the
		 *			faces of the mesh
		 * @param	vao the VAO of the mesh
		 * @note	the IBO and the VBOs must be already bound to the VAO with
		 *			its respective attribute indices
		 * @return	a reference to the current Mesh object */
		Mesh& setBuffers(
			std::vector<std::unique_ptr<VertexBuffer>>&& vbos,
			std::unique_ptr<IndexBuffer>&& ibo,
			std::unique_ptr<VertexArray>&& vao
		);

		/** Sets the bounds of the Mesh
		 *
		 * @param	minimum the minimum position of the mesh at each direction
		 * @param	maximum the maximum position of the mesh at each
		 *			direction
		 * @return	a reference to the current Mesh object */
		Mesh& setBounds(const glm::vec3& minimum, const glm::vec3& maximum);

		/** @return	the minimum and maximum position of the Mesh in each
		 *			direction */
		std::pair<glm::vec3, glm::vec3> getBounds() const;

		/** @return a reference to the VertexBuffers of the Mesh */
		const std::vector<std::unique_ptr<VertexBuffer>>& getVBOs() const;

		/** @return a reference to the IndexBuffer of the Mesh */
		const std::unique_ptr<IndexBuffer>& getIBO() const;

		/** @return a reference to the VertexArray of the Mesh */
		const std::unique_ptr<VertexArray>& getVAO() const;

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override;

		/** Binds the VAO of the Mesh */
		virtual void bind() const override;

		/** Unbinds the VAO of the Mesh */
		virtual void unbind() const override;
	};

}

#endif		// MESH_H
