#ifndef RENDERER_PARTICLES_H
#define RENDERER_PARTICLES_H

#include "Renderer3D.h"
#include "ParticleSystem.h"

namespace se::graphics {

	/**
	 * Class RendererParticles, it's a Renderer3D used for rendering
	 * ParticleSystems
	 */
	class RendererParticles : public Renderer3D
	{
	protected:	// Nested types
		using RenderablePassPair = std::pair<ParticleSystem*, Pass*>;

	protected:	// Attributes
		/** The submited ParticleSystems that are going to be drawn */
		std::vector<RenderablePassPair> mRenderQueue;

	public:		// Functions
		/** Creates a new RendererParticles
		 *
		 * @param	name the name of the new RendererParticles */
		RendererParticles(const std::string& name) : Renderer3D(name) {};

		/** Class destructor */
		virtual ~RendererParticles() = default;
	protected:
		/** @copydoc Renderer::sortQueue() */
		virtual void sortQueue() override;

		/** @copydoc Renderer::render(Context::Query&) */
		virtual void render(Context::Query& q) override;

		/** @copydoc Renderer::clearQueue() */
		virtual void clearQueue() override;

		/** @copydoc Renderer3D::submitRenderable3D(Renderable3D&, Pass&) */
		virtual
		void submitRenderable3D(Renderable3D& renderable, Pass& pass) override;
	};

}

#endif		// RENDERER_PARTICLES_H
