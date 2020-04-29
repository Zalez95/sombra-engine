#ifndef RENDERER_2D_H
#define RENDERER_2D_H

#include <vector>
#include <glm/glm.hpp>
#include "../../utils/FixedVector.h"
#include "../core/VertexArray.h"
#include "../core/VertexBuffer.h"
#include "../core/IndexBuffer.h"

namespace se::graphics {

	class Renderable2D;
	class Step2D;
	class Texture;


	/**
	 * Class Renderer2D, it's a Batch Renderer used for rendering 2D
	 * graphics elements. The Renderer2D draws the submitted Renderable2Ds
	 * ordered by their z-index.
	 */
	class Renderer2D
	{
	public:		// Nested types
		/** Holds the data of each of the vertices that the Batch can draw */
		struct BatchVertex
		{
			glm::vec2 position;						///< 2D position
			glm::vec2 texCoords;					///< Texture coordinates
			glm::vec4 color;						///< RGBA Color
			unsigned char textureId = kNoTexture;	///< Texture index
		};
	private:
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

			/** The Index Buffer Object of the Batch */
			IndexBuffer mIBO;

			/** The Vertex Array Object of the Batch */
			VertexArray mVAO;

			/** The 2D positions of the vertices of the Batch */
			std::vector<glm::vec2> mPositions;

			/** The texture coordinates of the vertices of the Batch */
			std::vector<glm::vec2> mTexCoords;

			/** The colors of the vertices of the Batch */
			std::vector<glm::vec4> mColors;

			/** The texture indices of the vertices of the Batch */
			std::vector<unsigned char> mTextureIds;

			/** The indices of the faces of the Batch */
			std::vector<unsigned short> mIndices;

		public:		// Functions
			/** Creates a new Batch
			 * @param	maxVertices the maximum number of vertices that can be
			 *			stored into the Batch
			 * @param	maxIndices the maximum number of indices that can be
			 *			stored into the Batch */
			Batch(std::size_t maxVertices, std::size_t maxIndices);

			/** @return	the number of vertices left until the Batch is full */
			std::size_t getVerticesLeft() const
			{ return mPositions.capacity() - mPositions.size(); };

			/** @return	the number of indices left until the Batch is full */
			std::size_t getIndicesLeft() const
			{ return mPositions.capacity() - mPositions.size(); };

			/** Submits the given vertices to the Batch
			 *
			 * @param	vertices a pointer to the vertices to submit
			 * @param	vertexCount the number of vertices to submit
			 * @param	indices a pointer to the indices to submit
			 * @param	indexCount the number of indices to submit */
			void submit(
				const BatchVertex* vertices, std::size_t vertexCount,
				const unsigned short* indices, std::size_t indexCount
			);

			/** Draws the Batch
			 *
			 * @note	after the Batch is drawn, the batch will be empty */
			void draw();
		};

		using RenderableStepPair = std::pair<Renderable2D*, Step2D*>;

	public:		// Attributes
		/** The maximum number of quads in each batch */
		static constexpr unsigned int kQuadsPerBatch = 1024;

		/** The maximum number of textures in each batch */
		static constexpr unsigned int kMaxTextures = 16;

		/** The value used for signaling that vertex doesn't use Textures */
		static constexpr unsigned char kNoTexture
			= static_cast<unsigned char>(-1);
	private:
		/** The Quad2D used for rendering all the renderables */
		Batch mBatch;

		/** The textures used for rendering the Batch */
		utils::FixedVector<Texture*, 16> mTextures;

		/** The Step2D used for rendering the Batch */
		Step2D* mStep;

		/** The submited Renderable2Ds that are going to be drawn */
		std::vector<RenderableStepPair> mRenderQueue;

	public:		// Functions
		/** Creates a new Renderer2D */
		Renderer2D() :
			mBatch(4 * kQuadsPerBatch, 6 * kQuadsPerBatch), mStep(nullptr) {};

		/** Submits the given Renderable2D for rendering
		 *
		 * @param	renderable the Renderable2D to submit for rendering
		 * @param	step the Step2D with which the Renderable2D will be drawn */
		void submit(Renderable2D& renderable, Step2D& step);

		/** Renders all the submitted Renderable2Ds */
		void render();

		/** Submits the given vertices to the Renderer2D
		 *
		 * @param	vertices a pointer to the vertices to submit
		 * @param	vertexCount the number of vertices to submit
		 * @param	indices a pointer to the indices to submit
		 * @param	indexCount the number of indices to submit
		 * @param	texture a pointer to the texture to submit */
		void submitVertices(
			BatchVertex* vertices, std::size_t vertexCount,
			const unsigned short* indices, std::size_t indexCount,
			Texture* texture = nullptr
		);
	private:
		/** Draws the batch with the current Step and clears the texture
		 * array of Texture uniforms */
		void drawBatch();
	};

}

#endif		// RENDERER_2D_H
