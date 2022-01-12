#include "se/graphics/core/Texture.h"
#include "se/graphics/2D/RenderableSprite.h"
#include "se/graphics/2D/Renderer2D.h"

namespace se::graphics {

	static constexpr unsigned short sQuadIndices[] = { 0, 2, 1, 1, 2, 3 };


	void RenderableSprite::submitVertices(Context::Query& q, Renderer2D& renderer) const
	{
		Renderer2D::BatchVertex vertices[] = {
			{ { mPosition.x, mPosition.y }, { 0.0f, 0.0f }, mColor },
			{ { mPosition.x + mSize.x, mPosition.y }, { 1.0f, 0.0f }, mColor },
			{ { mPosition.x, mPosition.y + mSize.y }, { 0.0f, 1.0f }, mColor },
			{ { mPosition.x + mSize.x, mPosition.y + mSize.y }, { 1.0f, 1.0f }, mColor }
		};
		renderer.submitVertices(q, vertices, 4, sQuadIndices, 6, q.getTBindable(mTexture));
	}

}
