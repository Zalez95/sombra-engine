#include <algorithm>
#include "se/graphics/Pass.h"
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/3D/Renderable3D.h"

namespace se::graphics {

	void Renderer3D::submit(Renderable& renderable, Pass& pass)
	{
		auto renderable3D = dynamic_cast<Renderable3D*>(&renderable);
		if (renderable3D) {
			mRenderQueue.emplace_back(renderable3D, &pass);
		}
	}


	void Renderer3D::render()
	{
		// Sort the render queue by Pass
		std::sort(
			mRenderQueue.begin(), mRenderQueue.end(),
			[](const RenderablePassPair& lhs, const RenderablePassPair& rhs) { return lhs.second < rhs.second; }
		);

		// Draw all the renderables
		const Pass* lastPass = nullptr;
		for (auto& [renderable, pass] : mRenderQueue) {
			if (pass != lastPass) {
				lastPass = pass;
				pass->bind();
			}

			renderable->bind(pass);
			renderable->draw();
		}
		mRenderQueue.clear();
	}

}
