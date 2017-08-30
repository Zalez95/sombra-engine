#ifndef RENDERER_2D_H
#define RENDERER_2D_H

#include <queue>
#include <GL/glew.h>
#include "Program2D.h"
#include "../buffers/VertexArray.h"
#include "../buffers/VertexBuffer.h"

namespace graphics {

	class Renderable2D;


	/**
	 * Class Renderer2D, it's a Forward Renderer used for rendering 2D
	 * graphics elements
	 */
	class Renderer2D
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
			static const GLfloat mPositions[];

			/** The number of vertices in the positions array */
			static const unsigned int mNumVertices = 4;

			/** The number of components in the positions array needed for each
			 * vertex */
			static const unsigned int mNumComponentsPerVertex = 2;

			/** The vertex buffer for the positions of the quad  of the renderer */
			VertexBuffer mPositionsBuffer;

			/** The Vertex Array Object of the Renderer */
			VertexArray mVAO;

		public:		// Functions
			/** Creates a new 2DQuad */
			Quad2D() :
				mPositionsBuffer(
					mPositions,
					mNumVertices * mNumComponentsPerVertex,
					mNumComponentsPerVertex
				)
			{ mVAO.addBuffer(&mPositionsBuffer, 0); };

			/** Class destructor */
			~Quad2D() {};

			/** @return	the number of vertices of the quad */
			inline unsigned int getNumVertices() const
			{ return mNumVertices; };

			/** Binds the VAO of the 2DQuad */
			inline void bindVAO() const { mVAO.bind(); };
		} mQuad;

		/** The Renderable2Ds that we want to render */
		std::queue<const Renderable2D*> mRenderable2Ds;

	public:		// Functions
		/** Creates a new Renderer2D
		 *
		 * @param	projectionMatrix the projectionMatrix of the renderer */
		Renderer2D() {};

		/** Class destructor */
		~Renderer2D() {};

		/** Submits the given Renderable 2D to the queue of Renderable2Ds to
		 * render
		 *
		 * @param	renderable2D a pointer to the Renderable2D that we want to
		 *			render */
		void submit(const Renderable2D* renderable2D);

		/** Renders the Renderable2Ds that currently are in the render queue
		 * 
		 * @note	after calling this method the render queue will be empty */
		void render();
	};

}

#endif		// RENDERER_2D_H
