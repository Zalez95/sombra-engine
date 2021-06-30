#include "se/graphics/ViewportResolutionNode.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::graphics {

	ViewportResolutionNode::ViewportResolutionNode(const std::string& name) :
		RenderNode(name), mX(0), mY(0), mWidth(0), mHeight(0)
	{
		addInput( std::make_unique<RNodeInput>("attach", this) );
		addOutput( std::make_unique<RNodeOutput>("attach", this) );
	}


	void ViewportResolutionNode::getViewportSize(int& x, int& y, std::size_t& width, std::size_t& height)
	{
		x = mX;
		y = mY;
		width = mWidth;
		height = mHeight;
	}


	void ViewportResolutionNode::setViewportSize(int x, int y, std::size_t width, std::size_t height)
	{
		mX = x;
		mY = y;
		mWidth = width;
		mHeight = height;
	}


	void ViewportResolutionNode::execute()
	{
		graphics::GraphicsOperations::setViewport(mX, mY, mWidth, mHeight);
	}

}
