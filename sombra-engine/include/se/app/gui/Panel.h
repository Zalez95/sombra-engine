#ifndef PANEL_H
#define PANEL_H

#include "ComposedComponent.h"
#include "../../graphics/2D/Renderable2D.h"
#include "../../graphics/2D/Layer2D.h"

namespace se::app {

	/**
	 * Class Panel, it's a visible GUI element used to hold and notify the
	 * IComponents added to it when it's updated
	 */
	class Panel : public ComposedComponent
	{
	private:	// Attributes
		/** The Renderable2D used for drawing of the Panel */
		graphics::Renderable2D mRenderable2D;

		/** A pointer to the Layer2D where @see Renderable2D will be submitted
		 * for drawing the Panel */
		graphics::Layer2D* mLayer2D;

	public:		// Functions
		/** Creates a new Panel */
		Panel(graphics::Layer2D* layer2D);

		/** Class destructor */
		virtual ~Panel();

		/** Sets the position of the Panel
		 *
		 * @param	position the new Position of the Panel */
		virtual void setPosition(const glm::vec2& position) override;

		/** Sets the size of the Panel
		 *
		 * @param	size the new Size of the Panel */
		virtual void setSize(const glm::vec2& size) override;

		/** Sets the z-index of the Panel
		 *
		 * @param	zIndex the new z-index of the Panel */
		virtual void setZIndex(unsigned char zIndex) override;

		/** Sets the color of the Panel
		 *
		 * @param	color the new color of the panel */
		void setColor(const glm::vec4& color);
	};

}

#endif		// PANEL_H
