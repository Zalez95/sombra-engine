#ifndef RENDERER_3D_H
#define RENDERER_3D_H

#include "../Renderer.h"

namespace se::graphics {

	class Renderable3D;


	/**
	 * Class Renderer3D, it's a Renderer used for rendering 3D Renderables.
	 */
	class Renderer3D : public Renderer
	{
	private:	// Nested types
		using RenderablePassPair = std::pair<Renderable3D*, Pass*>;

	private:	// Attributes
		/** The submited Renderable3Des that are going to be drawn */
		std::vector<RenderablePassPair> mRenderQueue;

	public:		// Functions
		/** Creates a new Renderer3D
		 *
		 * @param	name the name of the new Renderer3D */
		Renderer3D(const std::string& name) : Renderer(name) {};

		/** Class destructor */
		virtual ~Renderer3D() = default;

		/** @copydoc Renderer::submit(Renderable&, Pass&) */
		virtual void submit(Renderable& renderable, Pass& pass) override;

		/** @copydoc Renderer::render() */
		virtual void render() override;
	};

}

#endif		// RENDERER_3D_H
