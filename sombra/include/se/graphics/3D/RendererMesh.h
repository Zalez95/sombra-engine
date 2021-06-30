#ifndef RENDERER_MESH_H
#define RENDERER_MESH_H

#include "Renderer3D.h"
#include "RenderableMesh.h"

namespace se::graphics {

	/**
	 * Class RendererMesh, it's a Renderer3D used for rendering RenderableMeshes
	 */
	class RendererMesh : public Renderer3D
	{
	protected:	// Nested types
		using RenderablePassPair = std::pair<RenderableMesh*, Pass*>;

	protected:	// Attributes
		/** The submited RenderableMeshes that are going to be drawn */
		std::vector<RenderablePassPair> mRenderQueue;

	public:		// Functions
		/** Creates a new RendererMesh
		 *
		 * @param	name the name of the new RendererMesh */
		RendererMesh(const std::string& name) : Renderer3D(name) {};

		/** Class destructor */
		virtual ~RendererMesh() = default;
	protected:
		/** @copydoc Renderer::sortQueue() */
		virtual void sortQueue() override;

		/** @copydoc Renderer::render() */
		virtual void render() override;

		/** @copydoc Renderer::clearQueue() */
		virtual void clearQueue() override;

		/** @copydoc Renderer3D::submitRenderable3D(Renderable3D&, Pass&) */
		virtual
		void submitRenderable3D(Renderable3D& renderable, Pass& pass) override;
	};

}

#endif		// RENDERER_MESH_H
