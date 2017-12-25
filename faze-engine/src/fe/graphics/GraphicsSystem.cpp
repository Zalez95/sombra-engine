#include "fe/graphics/GraphicsSystem.h"
#include <algorithm>
#include <GL/glew.h>

namespace fe { namespace graphics {

	GraphicsSystem::GraphicsSystem()
	{
		// Enable depth-testing
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);		// Write if depth <= depth buffer
		glDepthRange(0.0f, 1.0f);	// The Z coordinate range is [0,1]

		// Enable face culling - tells OpenGL to not draw the faces that
		// cannot be seen
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);		// Render only the counter-clockwise faces

		// The Clear Color of the window
		glClearColor(0.0f, 0.05f, 0.1f, 1.0f);
	}


	void GraphicsSystem::addLayer(ILayer* layer)
	{
		if (layer) {
			mLayers.push_back(layer);
		}
	}


	void GraphicsSystem::removeLayer(ILayer* layer)
	{
		mLayers.erase(
			std::remove(mLayers.begin(), mLayers.end(), layer),
			mLayers.end()
		);
	}


	void GraphicsSystem::render() {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (ILayer* layer : mLayers) {
			layer->render();
		}
	}

}}
