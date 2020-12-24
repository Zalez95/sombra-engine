#include "se/graphics/core/Texture.h"
#include "se/graphics/core/GraphicsOperations.h"
#include "se/app/graphics/ShadowRenderer3D.h"

namespace se::app {

	ShadowRenderer3D::ShadowRenderer3D(const std::string& name) :
		FrustumRenderer3D(name), mShadowResolution(0)
	{
		addBindable(std::make_shared<graphics::SetOperation>(graphics::Operation::DepthTest));

		mIShadowTexBindable = addBindable(nullptr, false);
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("shadow", this, mIShadowTexBindable) );
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("shadow", this, mIShadowTexBindable) );
	}


	ShadowRenderer3D& ShadowRenderer3D::setShadowResolution(std::size_t shadowResolution)
	{
		mShadowResolution = shadowResolution;

		auto shadowMap = std::dynamic_pointer_cast<graphics::Texture>( getBindable(mIShadowTexBindable) );
		shadowMap->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth, mShadowResolution, mShadowResolution);

		return *this;
	}


	void ShadowRenderer3D::render()
	{
		int lastX, lastY;
		std::size_t lastWidth, lastHeight;
		se::graphics::GraphicsOperations::getViewport(lastX, lastY, lastWidth, lastHeight);

		graphics::GraphicsOperations::setViewport(0, 0, mShadowResolution, mShadowResolution);
		graphics::GraphicsOperations::setCullingMode(graphics::FaceMode::Front);

		FrustumRenderer3D::render();

		graphics::GraphicsOperations::setCullingMode(graphics::FaceMode::Back);
		graphics::GraphicsOperations::setViewport(lastX, lastY, lastWidth, lastHeight);
	}

}
