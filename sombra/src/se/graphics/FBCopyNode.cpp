#include "se/graphics/FBCopyNode.h"
#include "se/graphics/core/FrameBuffer.h"

namespace se::graphics {

	FBCopyNode::FBCopyNode(const std::string& name, const FrameBufferMask::Mask& mask) :
		BindableRenderNode(name), mMask(mask),
		mX0(0), mY0(0), mW0(0), mH0(0), mX1(0), mY1(0), mW1(0), mH1(0), mFilter(TextureFilter::Nearest)
	{
		mFrameBuffer1Index = addBindable();
		mFrameBuffer2Index = addBindable();

		addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("input1", this, mFrameBuffer1Index) );
		addInput( std::make_unique<BindableRNodeInput<FrameBuffer>>("input2", this, mFrameBuffer2Index) );
		addOutput( std::make_unique<BindableRNodeOutput<FrameBuffer>>("output", this, mFrameBuffer1Index) );
	}


	FBCopyNode& FBCopyNode::setDimensions1(std::size_t x, std::size_t y, std::size_t w, std::size_t h)
	{
		mX0 = x;
		mY0 = y;
		mW0 = w;
		mH0 = h;

		return *this;
	}


	FBCopyNode& FBCopyNode::setDimensions2(std::size_t x, std::size_t y, std::size_t w, std::size_t h)
	{
		mX1 = x;
		mY1 = y;
		mW1 = w;
		mH1 = h;

		return *this;
	}


	FBCopyNode& FBCopyNode::setFilter(TextureFilter filter)
	{
		mFilter = filter;
		return *this;
	}


	void FBCopyNode::execute(Context::Query& q)
	{
		auto fb1 = Context::TBindableRef<FrameBuffer>::from( getBindable(mFrameBuffer1Index) );
		auto fb2 = Context::TBindableRef<FrameBuffer>::from( getBindable(mFrameBuffer2Index) );
		q.getTBindable(fb1)->copy(*q.getTBindable(fb2), mMask, mX0, mY0, mW0, mH0, mX1, mY1, mW1, mH1, mFilter);
	}

}
