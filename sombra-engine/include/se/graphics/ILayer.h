#ifndef I_LAYER_H
#define I_LAYER_H

namespace se::graphics {

	/**
	 * Class ILayer. It's an interface that the GraphicsSystem will use to
	 * render all the Graphics elements.
	 */
	class ILayer
	{
	public:		// Functions
		/** Class destructor */
		virtual ~ILayer() = default;

		/** Draws the Layer */
		virtual void render() = 0;

		/** Sets the viewport size
		 *
		 * @param	width the new width of the layer viewport to render to
		 * @param	height the new height of the layer viewport to render to */
		virtual void setViewportSize(int width, int height) = 0;
	};

}

#endif		// I_LAYER_H
