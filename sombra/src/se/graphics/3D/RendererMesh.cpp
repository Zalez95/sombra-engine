#include <algorithm>
#include "se/graphics/Pass.h"
#include "se/graphics/3D/RendererMesh.h"

namespace se::graphics {

	void RendererMesh::sortQueue()
	{
		// Sort the render queue by Pass
		std::sort(
			mRenderQueue.begin(), mRenderQueue.end(),
			[](const RenderablePassPair& lhs, const RenderablePassPair& rhs) { return lhs.second < rhs.second; }
		);
	}


	void RendererMesh::render()
	{
		const Pass* lastPass = nullptr;
		for (auto& [renderable, pass] : mRenderQueue) {
			if (pass != lastPass) {
				lastPass = pass;
				pass->bind();
			}

			renderable->bind(pass);
			renderable->draw();
		}
	}


	void RendererMesh::clearQueue()
	{
		mRenderQueue.clear();
	}


	void RendererMesh::submitRenderable3D(Renderable3D& renderable, Pass& pass)
	{
		auto renderableMesh = dynamic_cast<RenderableMesh*>(&renderable);
		if (renderableMesh) {
			mRenderQueue.emplace_back(renderableMesh, &pass);
		}
	}

}
