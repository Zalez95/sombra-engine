#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <memory>
#include "Mesh.h"
#include "Particles.h"
#include "Renderable3D.h"
#include "../Context.h"

namespace se::graphics {

	/**
	 * Class ParticleSystem, it's a Renderable3D used for drawing particles
	 */
	class ParticleSystem : public Renderable3D
	{
	private:	// Attributes
		/** The Mesh of the ParticleSystem */
		Context::TBindableRef<Mesh> mMesh;

		/** The Particles data of the ParticleSystem */
		Context::TBindableRef<Particles> mParticles;

		/** The VAO of the ParticleSystem */
		std::unique_ptr<VertexArray> mVAO;

		/** If @see mVAO should be updated or not */
		bool mUpdateVAO = true;

		/** The type of primitive used for rendering each Particle */
		PrimitiveType mPrimitiveType;

		/** The number of Particle instances to draw */
		std::size_t mNumInstances = 0;

		/** The minimum position of the ParticleSystem at each direction in
		 * world space */
		glm::vec3 mMinimum = {};

		/** The maximum position of the ParticleSystem at each direction in
		 * world space */
		glm::vec3 mMaximum = {};

	public:		// Functions
		/** Creates a new ParticleSystem from the given data
		 *
		 * @param	mesh a reference to the Mesh of the ParticleSystem
		 * @param	particles  a reference to the Particles of the
		 *			ParticleSystem
		 * @param	primitiveType the type of primitive used for rendering each
		 *			Particle */
		ParticleSystem(
			const Context::TBindableRef<Mesh>& mesh = {},
			const Context::TBindableRef<Particles>& particles = {},
			PrimitiveType primitiveType = PrimitiveType::Triangle
		);
		ParticleSystem(const ParticleSystem& other);
		ParticleSystem(ParticleSystem&& other) = default;

		/** Class destructor */
		~ParticleSystem() = default;

		/** Assignment operator */
		ParticleSystem& operator=(const ParticleSystem& other);
		ParticleSystem& operator=(ParticleSystem&& other) = default;

		/** @return	the Mesh pointed by the ParticleSystem */
		const Context::TBindableRef<Mesh>& getMesh() const
		{ return mMesh; };

		/** Sets the Mesh pointed by the ParticleSystem
		 *
		 * @param	mesh a reference to the new Mesh of the ParticleSystem
		 * @return	a reference to the current ParticleSystem */
		ParticleSystem& setMesh(const Context::TBindableRef<Mesh>& mesh);

		/** @return	the Particles pointed by the ParticleSystem */
		const Context::TBindableRef<Particles>& getParticles() const
		{ return mParticles; };

		/** Sets the Particles pointed by the ParticleSystem
		 *
		 * @param	particles a reference to the new Particles of the
		 *			ParticleSystem
		 * @return	a reference to the current ParticleSystem */
		ParticleSystem& setParticles(
			const Context::TBindableRef<Particles>& particles
		);

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

		/** @copydoc Renderable3D::getBounds() */
		virtual std::pair<glm::vec3, glm::vec3> getBounds() const override
		{ return { mMinimum, mMaximum }; };

		/** Sets the bounds of the ParticleSystem
		 *
		 * @param	minimum the minimum position of the ParticleSystem
		 *			at each direction
		 * @param	maximum the maximum position of the ParticleSystem
		 *			at each direction */
		void setBounds(const glm::vec3& minimum, const glm::vec3& maximum);

		/** @copydoc Renderable::submit(Context::Query&) */
		virtual void submit(Context::Query& q) override;

		/** Draws all the particle instances
		 *
		 * @param	q the Context Query object used for accesing to the
		 *			Bindables */
		virtual void drawInstances(Context::Query& q);
	};

}

#endif		// PARTICLE_SYSTEM_H
