#ifndef RENDERER_TERRAIN_H
#define RENDERER_TERRAIN_H

#include <glm/glm.hpp>
#include "../core/VertexBuffer.h"
#include "../core/VertexArray.h"
#include "Renderer3D.h"
#include "RenderableTerrain.h"

namespace se::graphics {

	/**
	 * Class RendererTerrain, it's a Renderer3D used for rendering
	 * RenderableTerrains.
	 */
	class RendererTerrain : public Renderer3D
	{
	protected:	// Nested types
		using RenderablePassPair = std::pair<RenderableTerrain*, Pass*>;

		/** Struct Patch, holds the common graphics data of each patch of a
		 * Terrain */
		class Patch
		{
		private:	// Attributes
			/** The number of vertices of the patch */
			std::size_t mNumVertices;

			/** The VBO that holds the patch vertices position */
			Context::TBindableRef<VertexBuffer> mVBOXZPositions;

			/** The VBO that holds the location of the patch instances in local
			 * space */
			Context::TBindableRef<VertexBuffer> mVBOXZLocations;

			/** The VBO that holds the lods of the patch instances */
			Context::TBindableRef<VertexBuffer> mVBOLods;

			/** The VAO of the patch */
			Context::TBindableRef<VertexArray> mVAO;

			/** The number of instances to render */
			std::size_t mInstanceCount;

			/** The location of the patch instances in local space */
			std::vector<glm::vec2> mXZLocations;

			/** The lods of the patch instances */
			std::vector<int> mLods;

		public:		// Functions
			/** Creates a new Patch
			 *
			 * @param	context the graphics Context used for creating the
			 *			Bindables
			 * @param	vertices a pointer to the vertices of the patch
			 * @param	numVertices the number of vertices */
			Patch(
				Context& context, const float* vertices, std::size_t numVertices
			);

			/** Adds an instance to draw
			 *
			 * @param	nodeLocation the location of the instance in local space
			 * @param	lod the lod of the instance */
			void submitInstance(const glm::vec2& nodeLocation, int lod);

			/** Draws all the instances of the patch
			 *
			 * @param	q the Context Query object used for accesing to the
			 *			Bindables */
			void drawInstances(Context::Query& q);
		};

	protected:	// Attributes
		/** The submited RenderableTerrains that are going to be drawn */
		std::vector<RenderablePassPair> mRenderQueue;

		/** The patch to render with all the vertices */
		Patch mNormal;

		/** The patches to render with some side face missing used for
		 * morphing between different lods */
		Patch mBottom, mTop, mLeft, mRight;

		/** The patches to render with some corner faces missing used for
		 * morphing between different lods */
		Patch mBottomLeft, mBottomRight, mTopLeft, mTopRight;

	public:		// Functions
		/** Creates a new RendererTerrain
		 *
		 * @param	name the name of the new RendererTerrain
		 * @param	context the graphics Context used for creating the
		 *			Bindables */
		RendererTerrain(const std::string& name, Context& context);

		/** Class destructor */
		virtual ~RendererTerrain() = default;
	protected:
		/** @copydoc Renderer::sortQueue() */
		virtual void sortQueue() override;

		/** @copydoc Renderer::render(Context::Query&) */
		virtual void render(Context::Query& q) override;

		/** @copydoc Renderer::clearQueue() */
		virtual void clearQueue() override;

		/** @copydoc Renderer3D::submitRenderable3D(Renderable3D&, Pass&) */
		virtual
		void submitRenderable3D(Renderable3D& renderable, Pass& pass) override;

		/** Set the given leaf node or its descendants as instances of a patch
		 * to render
		 *
		 * @param	node the QuadTree Node to draw
		 * @param	parentLocation the location of the parent node relative
		 *			to the root node */
		void submitNode(
			const QuadTree::Node& node,
			const glm::vec2& parentLocation
		);
	};

}

#endif		// RENDERER_TERRAIN_H
