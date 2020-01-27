#ifndef RENDERER_2D_H
#define RENDERER_2D_H

#include <vector>
#include "Program2D.h"
#include "../../utils/FixedVector.h"
#include "../core/VertexArray.h"
#include "../core/VertexBuffer.h"

namespace se::graphics {

	class Renderable2D;
	class RenderableText;


	/**
	 * Class Renderer2D, it's a Batch Renderer used for rendering 2D
	 * graphics elements. The Renderer2D draws the elements in the given order
	 * without taking in count its z-index.
	 */
	class Renderer2D
	{
	private:	// Nested types
		typedef std::shared_ptr<Texture> TextureSPtr;

		/** Holds the data of each of the vertices that the Batch can draw */
		struct BatchVertex
		{
			glm::vec2 position;			///< 2D position of the vertex
			glm::vec2 texCoords;		///< Texture coordinates of the vertex
			glm::vec4 color;			///< RGBA Color of the vertex
			unsigned char textureId;	///< Texture index of the vertex
		};

		/**
		 * Struct Batch, holds all the needed data for rendering the 2D elements
		 */
		class Batch
		{
		private:	// Attributes
			/** The VBO that holds the vertex positions of the Batch */
			VertexBuffer mVBOPositions;

			/** The VBO that holds the vertex texture coordiantes of the
			 * Batch */
			VertexBuffer mVBOTexCoords;

			/** The VBO that holds the vertex colors of the Batch */
			VertexBuffer mVBOColors;

			/** The VBO that holds the vertex texture ids to use with the
			 * Batch */
			VertexBuffer mVBOTextureIds;

			/** The Vertex Array Object of the Batch */
			VertexArray mVAO;

			/** The number of vertices in the Batch */
			std::size_t mVertexCount;

			/** The 2D positions of the vertices of the Batch */
			std::vector<glm::vec2> mPositions;

			/** The texture coordinates of the vertices of the Batch */
			std::vector<glm::vec2> mTexCoords;

			/** The colors of the vertices of the Batch */
			std::vector<glm::vec4> mColors;

			/** The texture indices of the vertices of the Batch */
			std::vector<unsigned char> mTextureIds;

		public:		// Functions
			/** Creates a new Batch */
			Batch();

			/** Submits the given vertices to the Batch
			 *
			 * @param	vertices a pointer to the vertices to submit
			 * @param	vertexCount the number of vertices to submit */
			void submit(const BatchVertex* vertices, std::size_t vertexCount);

			/** Draws the Batch
			 *
			 * @note	after the Batch is drawn, the batch will be empty */
			void draw();
		};

	private:	// Attributes
		/** The Program of the renderer */
		Program2D mProgram;

		/** The Quad2D used for rendering all the renderables */
		Batch mBatch;

		/** The textures used for rendering the Batch */
		utils::FixedVector<TextureSPtr, 16> mTextures;

	public:		// Functions
		/** Creates a new Renderer2D */
		Renderer2D();

		/** Class destructor */
		~Renderer2D();

		/** Prepares the Renderer2D for rendering
		 *
		 * @param	projectionMatrix the matrix to use as Projection matrix in
		 *			the shaders */
		void start(const glm::mat4& projectionMatrix);

		/** Submits the given Renderable2D to the renderer
		 *
		 * @param	renderable2D a pointer to the Renderable2D that we
		 *			want to render */
		void submit(const Renderable2D* renderable2D);

		/** Submits the given RenderableText to the renderer
		 *
		 * @param	renderableText a pointer to the RenderableText that we
		 *			want to render */
		void submit(const RenderableText* renderableText);

		/** Renders the Renderables that are still in the batch
		 *
		 * @note	after calling this method the batch will be empty */
		void end();
	private:
		/** Adds the given texture to the array of Texture uniforms (if isn't
		 * already inside)
		 *
		 * @param	texture a pointer to the texture to add
		 * @return	the index of the texture in the array of Texture uniforms
		 * @note	if the array of Texture uniforms is full the Batch will be
		 *			rendered */
		unsigned char addTexture(TextureSPtr texture);

		/** Draws the batch and clears the texture array of Texture uniforms */
		void drawBatch();
	};

}

#endif		// RENDERER_2D_H
