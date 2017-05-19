#ifndef RENDERER_2D_H
#define RENDERER_2D_H

#include <queue>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "../Program.h"
#include "../buffers/VertexArray.h"
#include "../buffers/VertexBuffer.h"

namespace graphics {

	class Renderable2D;


	/**
	 * Class Renderer2D, it's a Forward Renderer used for rendering 2D 2D
	 * graphics elements
	 */
	class Renderer2D
	{
	private:	// Nested Types
		/** Struct UniformLocations, it holds the uniform variables location
		 * so we don't have to get them in each render call */
		struct UniformLocations
		{
			GLuint u_ModelMatrix;
		};

		/** Class 2DQuad, it holds all the needed data for rendering the 2D
		 * elements */
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
		};

	private:	// Attributes
		/** The Program of the renderer */
		Program* mProgram;

		/** The locations of uniform variables in the shader */
		UniformLocations mUniformLocations;

		/** The Renderables that we want to render */
		std::queue<const Renderable2D*> mRenderable2Ds;

		/** The quad needed for rendering all the 2D elements */
		Quad2D mQuad;

	public:		// Functions
		/** Creates a new Renderer2D and sets all the GL data like depth
		 * testing, face culling and the window clear color
		 *
		 * @param	program the Program with which the renderer will render */
		Renderer2D();

		/** Class destructor */
		~Renderer2D();

		/** Submits the given Renderable 2D to the queue of Renderable2Ds to
		 * render
		 *
		 * @param	renderable a pointer to the Renderable2D that we want to
		 *			render */
		inline void submit(const Renderable2D* renderable)
		{ mRenderable2Ds.push(renderable); };

		/** Renders the Renderable2Ds that currently are in the render queue
		 * 
		 * @note	after calling this method the render queue will be empty */
		void render();
	};

}

#endif		// RENDERER_2D_H
