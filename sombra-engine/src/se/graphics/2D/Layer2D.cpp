#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "se/graphics/2D/Layer2D.h"

namespace se::graphics {

	void Layer2D::addRenderable2D(const Renderable2D* renderable2D)
	{
		if (renderable2D) {
			mRenderable2Ds.push_back(renderable2D);
		}
	}


	void Layer2D::removeRenderable2D(const Renderable2D* renderable2D)
	{
		mRenderable2Ds.erase(
			std::remove(mRenderable2Ds.begin(), mRenderable2Ds.end(), renderable2D),
			mRenderable2Ds.end()
		);
	}


	void Layer2D::render()
	{
		for (const Renderable2D* renderable2D : mRenderable2Ds)
			mRenderer2D.submit(renderable2D);
		mRenderer2D.render();
	}

}
