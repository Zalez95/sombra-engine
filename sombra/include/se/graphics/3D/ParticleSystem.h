#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <memory>
#include "Mesh.h"
#include "Renderable3D.h"

namespace se::graphics {

	/**
	 * Class ParticleSystem, it's a Renderable3D used for drawing particles
	 */
	class ParticleSystem : public Renderable3D
	{
	public:		// Nested types
		using MeshSPtr = std::shared_ptr<Mesh>;

	private:	// Attributes
		/** The Mesh of the ParticleSystem */
		MeshSPtr mMesh;

		/** The per instance vertex buffers of the ParticleSystem */
		std::vector<VertexBuffer> mInstanceVBOs;

		/** The VAO of the ParticleSystem */
		VertexArray mVAO;

		/** The type of primitive used for rendering each Particle */
		PrimitiveType mPrimitiveType;

		/** The number of Particle instances to draw */
		std::size_t mNumInstances;

	public:		// Functions
		/** Creates a new ParticleSystem from the given data
		 *
		 * @param	mesh a pointer to the Mesh of the ParticleSystem
		 * @param	primitiveType the type of primitive used for rendering each
		 *			Particle */
		ParticleSystem(
			MeshSPtr mesh = nullptr,
			PrimitiveType primitiveType = PrimitiveType::Triangle
		) : mPrimitiveType(primitiveType), mNumInstances(0) { setMesh(mesh); };

		/** @return	the Mesh pointed by the ParticleSystem */
		MeshSPtr getMesh() const { return mMesh; };

		/** Sets the Mesh pointed by the ParticleSystem
		 *
		 * @param	mesh a pointer to the new Mesh pointed by the
		 *			ParticleSystem
		 * @return	a reference to the current ParticleSystem */
		ParticleSystem& setMesh(MeshSPtr mesh);

		/** @return	a reference to the per instance VertexBuffers of the
		 *			ParticleSystem */
		const std::vector<VertexBuffer>& getInstanceVBOs() const
		{ return mInstanceVBOs; };

		/** @return	a reference to the per instance VertexBuffers of the
		 *			ParticleSystem */
		std::vector<VertexBuffer>& getInstanceVBOs()
		{ return mInstanceVBOs; };

		/** @return a reference to the VertexArray of the Mesh */
		const VertexArray& getVAO() const { return mVAO; };

		/** @return a reference to the VertexArray of the Mesh */
		VertexArray& getVAO() { return mVAO; };

		/** @return	the primitive type used for rendering by the
		 *			ParticleSystem */
		PrimitiveType getPrimitiveType() const { return mPrimitiveType; };

		/** @return	the number of Particle instances to draw */
		std::size_t getNumInstances() const { return mNumInstances; };

		/** Sets the number of Particle instances to draw
		 *
		 * @param	numInstances the new number of Particle instances */
		void setNumInstances(std::size_t numInstances)
		{ mNumInstances = numInstances; };

		/** Sets the bounds of the ParticleSystem
		 *
		 * @param	minimum the minimum position of the ParticleSystem
		 *			at each direction
		 * @param	minimum the maximum position of the ParticleSystem
		 *			at each direction */
		void setBounds(const glm::vec3& minimum, const glm::vec3& maximum);

		/** Draws all the particle instances */
		virtual void drawInstances();
	};

}

#endif		// PARTICLE_SYSTEM_H
