#ifndef LAYER_2D_H
#define LAYER_2D_H

#include <array>
#include <vector>
#include "../ILayer.h"
#include "Renderer2D.h"

namespace se::graphics {

	/**
	 * Class Layer2D, it's a class that holds all the 2D elements that must be
	 * rendered
	 */
	class Layer2D : public ILayer
	{
	private:	// Nested types
		template <typename T, std::size_t S>
		using ArrayOfVectors = std::array<std::vector<T>, S>;

	private:	// Attributes
		/** Tha maximum value of the z-index of the Renderables */
		static constexpr std::size_t kMaxZIndex = 256;

		/** The Renderer used by the layer to render the Scene */
		Renderer2D mRenderer2D;

		/** The size of the viewport */
		glm::uvec2 mViewportSize;

		/** The projection matrix to use in the shaders */
		glm::mat4 mProjectionMatrix;

		/** The Renderable2Ds that the layer must render, from lowest to
		 * highest z-index */
		ArrayOfVectors<const Renderable2D*, kMaxZIndex> mRenderable2Ds;

		/** The RenderableTexts that the layer must render, from lowest to
		 * highest z-index */
		ArrayOfVectors<const RenderableText*, kMaxZIndex> mRenderableTexts;

	public:		// Functions
		/** Adds the given Renderable to the Layer so it will be rendered
		 *
		 * @param	renderable2D a pointer to the Renderable2D to add
		 * @param	zIndex determines the drawing order of the Renderable2D,
		 *			so it can appear on top of other Renderable2Ds/Texts
		 *			independently of when it was added to the Layer2D. The
		 *			smaller the value the further back it will appear. */
		void addRenderable2D(
			const Renderable2D* renderable2D,
			unsigned char zIndex
		);

		/** Removes the given Renderable from the Layer so it will no longer
		 * be rendered
		 *
		 * @param	renderable2D a pointer to the Renderable2D to remove
		 * @param	zIndex the z-index with which the Renderable2D was added
		 *			to the Layer2D */
		void removeRenderable2D(
			const Renderable2D* renderable2D,
			unsigned char zIndex
		);

		/** Adds the given Renderable to the Layer so it will be rendered
		 *
		 * @param	renderableText a pointer to the RenderableText to add
		 * @param	zIndex determines the drawing order of the RenderableText,
		 *			so it can appear on top of other Renderable2Ds/Texts
		 *			independently of when it was added to the Layer2D. The
		 *			smaller the value the further back it will appear. */
		void addRenderableText(
			const RenderableText* renderableText,
			unsigned char zIndex
		);

		/** Removes the given Renderable from the Layer so it will no longer
		 * be rendered
		 *
		 * @param	renderableText a pointer to the RenderableText to remove
		 * @param	zIndex the z-index with which the RenderableText was added
		 *			to the Layer2D */
		void removeRenderableText(
			const RenderableText* renderableText,
			unsigned char zIndex
		);

		/** Draws the scene */
		void render() override;

		/** @return	the viewport size */
		const glm::uvec2& getViewportSize() const { return mViewportSize; };

		/** Sets the viewport size
		 *
		 * @param	viewportSize the new size of the layer viewport to render to
		 * @note	the origin of the render viewport is located at the
		 *			top-left corner */
		virtual void setViewportSize(const glm::uvec2& viewportSize) override;
	};

}

#endif		// LAYER_2D_H
