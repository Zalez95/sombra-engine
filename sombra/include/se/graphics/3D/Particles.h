#ifndef PARTICLES_H
#define PARTICLES_H

#include <vector>
#include <memory>
#include "../core/VertexBuffer.h"
#include "../core/VertexArray.h"

namespace se::graphics {

	/**
	 * Class Particles, it holds the buffers that creates the 3D Particles
	 */
	class Particles : public Bindable
	{
	private:	// Attributes
		/** The vertex buffers of the Particles */
		std::vector<std::unique_ptr<VertexBuffer>> mVBOs;

		/** The VAO of the Particles */
		std::unique_ptr<VertexArray> mVAO;

	public:		// Functions
		/** Sets the buffers of the Particles
		 *
		 * @param	vbos the vertex buffers that cointain all the vertex data of
		 *			the Particles
		 * @param	vao the VAO of the mesh
		 * @note	the VBOs must be already bound to the VAO with its
		 *			respective attribute indices
		 * @return	a reference to the current Particles object */
		Particles& setBuffers(
			std::vector<std::unique_ptr<VertexBuffer>>&& vbos,
			std::unique_ptr<VertexArray>&& vao
		);

		/** @return a reference to the VertexBuffers of the Particles */
		const std::vector<std::unique_ptr<VertexBuffer>>& getVBOs() const;

		/** @return a reference to the VertexArray of the Particles */
		const std::unique_ptr<VertexArray>& getVAO() const;

		/** @copydoc Bindable::clone() */
		virtual std::unique_ptr<Bindable> clone() const override;

		/** Binds the VAO of the Particles */
		virtual void bind() const override;

		/** Unbinds the VAO of the Particles */
		virtual void unbind() const override;
	};

}

#endif		// PARTICLES_H
