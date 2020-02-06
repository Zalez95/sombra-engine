#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "se/graphics/2D/Layer2D.h"
#include "se/graphics/2D/Renderable2D.h"
#include "se/graphics/2D/RenderableText.h"

namespace se::graphics {

	void Layer2D::addRenderable2D(const Renderable2D* renderable2D, unsigned char zIndex)
	{
		if (renderable2D) {
			mRenderable2Ds[zIndex].push_back(renderable2D);
		}
	}


	void Layer2D::removeRenderable2D(const Renderable2D* renderable2D, unsigned char zIndex)
	{
		if (renderable2D) {
			mRenderable2Ds[zIndex].erase(
				std::remove(mRenderable2Ds[zIndex].begin(), mRenderable2Ds[zIndex].end(), renderable2D),
				mRenderable2Ds[zIndex].end()
			);
		}
	}


	void Layer2D::addRenderableText(const RenderableText* renderableText, unsigned char zIndex)
	{
		if (renderableText) {
			mRenderableTexts[zIndex].push_back(renderableText);
		}
	}


	void Layer2D::removeRenderableText(const RenderableText* renderableText, unsigned char zIndex)
	{
		if (renderableText) {
			mRenderableTexts[zIndex].erase(
				std::remove(mRenderableTexts[zIndex].begin(), mRenderableTexts[zIndex].end(), renderableText),
				mRenderableTexts[zIndex].end()
			);
		}
	}


	void Layer2D::render()
	{
		mRenderer2D.start(mProjectionMatrix);

		for (std::size_t zIndex = 0; zIndex < kMaxZIndex; ++zIndex) {
			for (const Renderable2D* renderable2D : mRenderable2Ds[zIndex]) {
				mRenderer2D.submit(renderable2D);
			}

			for (const RenderableText* renderableText : mRenderableTexts[zIndex]) {
				mRenderer2D.submit(renderableText);
			}
		}

		mRenderer2D.end();
	}


	void Layer2D::setViewportSize(const glm::uvec2& viewportSize)
	{
		mViewportSize = viewportSize;
		mProjectionMatrix = glm::ortho(0.0f, static_cast<float>(mViewportSize.x), static_cast<float>(mViewportSize.y), 0.0f, -1.0f, 1.0f);
	}

}
