#ifndef RENDERER_TEXT_H
#define RENDERER_TEXT_H

#include <queue>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "../Program.h"
#include "../buffers/VertexArray.h"
#include "../buffers/VertexBuffer.h"

namespace graphics {

	class RenderableText;


	/**
	 * Class RendererText, it's a Forward Renderer used for rendering texts
	 */
	class RendererText
	{
	private:	// Nested Types
		/** Struct UniformLocations, it holds the uniform variables location
		 * so we don't have to get them in each render call */
		struct UniformLocations
		{
//			GLuint u_ModelMatrix;
		};

	private:	// Attributes
		/** The Program of the renderer */
		Program* mProgram;

		/** The locations of uniform variables in the shader */
		UniformLocations mUniformLocations;

		/** The RenderableTexts that we want to render */
		std::queue<const RenderableText*> mRenderableTexts;

	public:		// Functions
		/** Creates a new RendererText
		 *
		 * @param	program the Program with which the renderer will render */
		RendererText();

		/** Class destructor */
		~RendererText();

		/** Submits the given Renderable 2D to the queue of RenderableTexts to
		 * render
		 *
		 * @param	renderable a pointer to the RenderableText that we want to
		 *			render */
		inline void submit(const RenderableText* renderable)
		{ mRenderableTexts.push(renderable); };

		/** Renders the RenderableTexts that currently are in the render queue
		 * 
		 * @note	after calling this method the render queue will be empty */
		void render();
	};

}

#endif		// RENDERER_TEXT_H
