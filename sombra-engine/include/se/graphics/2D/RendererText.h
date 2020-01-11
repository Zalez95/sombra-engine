#ifndef RENDERER_TEXT_H
#define RENDERER_TEXT_H

#include <queue>
#include "Program2D.h"
#include "../core/VertexArray.h"
#include "../core/VertexBuffer.h"

namespace se::graphics {

	class RenderableText;


	/**
	 * Class RendererText, it's a Forward Renderer used for rendering texts
	 */
	class RendererText
	{
	private:	// Attributes
		/** The Program of the renderer */
		Program2D mProgram;

		/** Holds all the needed data for rendering the 2D elements */
		class Quad2D
		{
		private:	// Attributes
			/** The positions of the only quad needed for rendering the 2D
			 * entities */
			static constexpr float kPositions[] = {
				0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f
			};

			/** The number of vertices in the positions array */
			static constexpr int kNumVertices = 4;

			/** The number of components in the positions array needed for each
			 * vertex */
			static constexpr int kNumComponentsPerVertex = 2;

			/** The vertex buffer for the positions of the quad  of the renderer */
			VertexBuffer mPositionsBuffer;

			/** The Vertex Array Object of the Renderer */
			VertexArray mVAO;

		public:		// Functions
			/** Creates a new 2DQuad */
			Quad2D();

			/** @return	the number of vertices of the quad */
			inline int getNumVertices() const { return kNumVertices; };

			/** Binds the VAO of the 2DQuad */
			inline void bind() const { mVAO.bind(); };

			/** Unbinds the VAO of the 2DQuad */
			inline void unbind() const { mVAO.unbind(); };
		} mQuad;

		/** The RenderableTexts that we want to render */
		std::queue<const RenderableText*> mRenderableTexts;

	public:		// Functions
		/** Creates a new RendererText */
		RendererText();

		/** Class destructor */
		~RendererText();

		/** Submits the given RenderableTexts to the queue of RenderableTexts to
		 * render
		 *
		 * @param	renderableText a pointer to the RenderableText that we want
		 *			to render */
		void submit(const RenderableText* renderableText);

		/** Renders the RenderableTexts that currently are in the render queue
		 *
		 * @param	projectionMatrix the matrix to use as Projection matrix in
		 *			the shaders
		 * @note	after calling this method the render queue will be empty */
		void render(const glm::mat4& projectionMatrix);
	};

}

#endif		// RENDERER_TEXT_H
