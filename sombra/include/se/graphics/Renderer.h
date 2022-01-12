#ifndef RENDERER_H
#define RENDERER_H

#include "BindableRenderNode.h"

namespace se::graphics {

	class Pass;
	class Renderable;


	/**
	 * Class Renderer, it's BindableRenderNode that can draw Renderables.
	 * Every Renderer has a Framebuffer input and output called "target"
	 */
	class Renderer : public BindableRenderNode
	{
	public:		// Functions
		/** Creates a new Renderer
		 *
		 * @param	name the name of the new Renderer */
		Renderer(const std::string& name);

		/** Class destructor */
		virtual ~Renderer() = default;

		/** @copydoc RenderNode::execute(Context::Query&) */
		virtual void execute(Context::Query& q) override;

		/** Submits the given Renderable for rendering
		 *
		 * @param	renderable the Renderable to submit for rendering
		 * @param	pass the Pass with which the Renderable will be drawn */
		virtual void submit(Renderable& renderable, Pass& pass) = 0;
	protected:
		/** Sorts the queue with all the submitted Renderables */
		virtual void sortQueue() = 0;

		/** Renders all the Renderables submitted to the Renderer Queue
		 *
		 * @param	q the Context Query object used for accesing to the
		 *			Bindables */
		virtual void render(Context::Query& q) = 0;

		/** Clears the queue with all the submitted Renderables */
		virtual void clearQueue() = 0;
	};

}

#endif		// RENDERER_H
