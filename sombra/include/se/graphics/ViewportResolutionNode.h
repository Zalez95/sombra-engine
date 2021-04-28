#ifndef VIEWPORT_RESOLUTION_NODE_H
#define VIEWPORT_RESOLUTION_NODE_H

#include "RenderNode.h"

namespace se::graphics {

	/**
	 * Class ViewportResolutionNode, it's RenderNode that can change the
	 * viewport origin and size. It has an "attach" RNodeInput and RNodeOutput
	 * connectors where other RenderNodes can be attached
	 */
	class ViewportResolutionNode : public RenderNode
	{
	private:	// Attributes
		/** The X and Y position of the viewport origin */
		int mX, mY;

		/** The width and height of the viewport */
		std::size_t mWidth, mHeight;

	public:		// Functions
		/** Creates a new ViewportResolutionNode
		 *
		 * @param	name the name of the new ViewportResolutionNode */
		ViewportResolutionNode(const std::string& name);

		/** Class destructor */
		virtual ~ViewportResolutionNode() = default;

		/** Sets the viewport coordinates and size to set when @see execute is
		 * called
		 *
		 * @param	x the origin (lower-left) of the viewport in the X axis
		 * @param	y the origin (lower-left) of the viewport in the Y axis
		 * @param	width the width of the viewport
		 * @param	height the height of the viewport */
		void setViewportSize(
			int x, int y, std::size_t width, std::size_t height
		);

		/** @copydoc RenderNode::execute() */
		virtual void execute() override;
	};

}

#endif		// VIEWPORT_RESOLUTION_NODE_H
