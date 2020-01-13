#ifndef LAYER_2D_H
#define LAYER_2D_H

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
	private:	// Attributes
		/** The Renderer used by the layer to render the Scene */
		Renderer2D mRenderer2D;

		/** The projection matrix to use in the shaders */
		glm::mat4 mProjectionMatrix;

		/** The 3D renderables that the layer must render */
		std::vector<const Renderable2D*> mRenderable2Ds;

		/** The 3D renderables that the layer must render */
		std::vector<const RenderableText*> mRenderableTexts;

	public:		// Functions
		/** Adds the given Renderable to the Layer so it will be rendered
		 *
		 * @param	renderable2D a pointer to the Renderable2D to add */
		void addRenderable2D(const Renderable2D* renderable2D);

		/** Removes the given Renderable from the Layer so it will no longer
		 * be rendered
		 *
		 * @param	renderable2D a pointer to the Renderable2D to remove */
		void removeRenderable2D(const Renderable2D* renderable2D);

		/** Adds the given Renderable to the Layer so it will be rendered
		 *
		 * @param	renderableText a pointer to the RenderableText to add */
		void addRenderableText(const RenderableText* renderableText);

		/** Removes the given Renderable from the Layer so it will no longer
		 * be rendered
		 *
		 * @param	renderableText a pointer to the RenderableText to remove */
		void removeRenderableText(const RenderableText* renderableText);

		/** Draws the scene */
		void render() override;

		/** Sets the viewport size
		 *
		 * @param	width the new width of the layer viewport to render to
		 * @param	height the new height of the layer viewport to render to
		 * @note	the origin of the render viewport is located at the
		 *			top-left corner */
		virtual void setViewportSize(int width, int height) override;
	};

}

#endif		// LAYER_2D_H
