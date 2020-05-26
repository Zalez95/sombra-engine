#ifndef RENDERER_H
#define RENDERER_H

#include "BindableRenderNode.h"

namespace se::graphics {

	class Pass;
	class Renderable;


	/**
	 * Class Renderer, it's RenderNode that can draw Renderables
	 */
	class Renderer : public BindableRenderNode
	{
	public:		// Functions
		/** Creates a new Renderer
		 *
		 * @param	name the name of the new Renderer */
		Renderer(const std::string& name) : BindableRenderNode(name) {};

		/** Class destructor */
		virtual ~Renderer() = default;

		/** Executes the current RenderNode */
		virtual void execute() override { bind(); render(); };

		/** Submits the given Renderable for rendering
		 *
		 * @param	renderable the Renderable to submit for rendering
		 * @param	pass the Pass with which the Renderable will be drawn */
		virtual void submit(Renderable& renderable, Pass& pass) = 0;

		/** Renders all the submitted Renderables */
		virtual void render() = 0;
	};

}

#endif		// RENDERER_H
