#include <algorithm>
#include "se/graphics/Pass.h"
#include "se/graphics/3D/RendererParticles.h"

namespace se::graphics {

	void RendererParticles::render()
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
			renderable->drawInstances();
		}
		mRenderQueue.clear();
	}

// Private functions
	void RendererParticles::submitRenderable3D(Renderable3D& renderable, Pass& pass)
	{
		auto particleSystem = dynamic_cast<ParticleSystem*>(&renderable);
		if (particleSystem) {
			mRenderQueue.emplace_back(particleSystem, &pass);
		}
	}

}
