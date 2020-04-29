#include <algorithm>
#include "se/graphics/3D/Renderer3D.h"
#include "se/graphics/3D/Step3D.h"
#include "se/graphics/3D/Renderable3D.h"

namespace se::graphics {

	void Renderer3D::submit(Renderable3D& renderable, Step3D& step)
	{
		mRenderQueue.emplace_back(&renderable, &step);
	}


	void Renderer3D::render()
	{
		// Sort the render queue by Step
		std::sort(
			mRenderQueue.begin(), mRenderQueue.end(),
			[](const RenderableStepPair& lhs, const RenderableStepPair& rhs) { return lhs.second < rhs.second; }
		);

		// Draw all the renderables
		const Step3D* lastStep = nullptr;
		for (auto& [renderable, step] : mRenderQueue) {
			if (step != lastStep) {
				lastStep = step;
				step->bind();
			}

			renderable->bind();
			renderable->draw();
		}
		mRenderQueue.clear();
	}

}
