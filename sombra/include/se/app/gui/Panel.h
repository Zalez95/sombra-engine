#ifndef PANEL_H
#define PANEL_H

#include "../../graphics/2D/RenderableSprite.h"
#include "../Repository.h"
#include "ComposedComponent.h"

namespace se::app {

	class GUIManager;


	/**
	 * Class Panel, it's a visible GUI element used for holding and notifying
	 * the IComponents added to it when it's updated
	 */
	class Panel : public ComposedComponent
	{
	private:	// Attributes
		/** A pointer to the GUIManager used for drawing the Panel */
		GUIManager* mGUIManager;

		/** The RenderableSprite used for drawing of the Panel */
		graphics::RenderableSprite mSprite;

		/** The technique of the Panel */
		Repository::ResourceRef<graphics::Technique> mTechnique;

	public:		// Functions
		/** Creates a new Panel
		 *
		 * @param	guiManager a pointer to the GUIManager used for drawing
		 *			the Panel */
		Panel(GUIManager* guiManager);
		Panel(const Panel& other);
		Panel(Panel&& other) = default;

		/** Class destructor */
		virtual ~Panel();

		/** Assignment operator */
		Panel& operator=(const Panel& other);
		Panel& operator=(Panel&& other) = default;

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

		/** Sets the Panel visibility on/off
		 *
		 * @param	isVisible if Panel must be shown or not */
		virtual void setVisibility(bool isVisible) override;

		/** Sets the color of the Panel
		 *
		 * @param	color the new color of the panel */
		void setColor(const glm::vec4& color);
	};

}

#endif		// PANEL_H
