#include <stdexcept>
#include <algorithm>
#include "se/graphics/GLWrapper.h"
#include "se/graphics/GraphicsSystem.h"

namespace se::graphics {

	GraphicsSystem::GraphicsSystem()
	{
		glewExperimental = true;
		if (glewInit() != GLEW_OK) {
			throw std::runtime_error("Failed to initialize GLEW");
		}

		// Enable depth-testing
		GL_WRAP( glEnable(GL_DEPTH_TEST) );
		GL_WRAP( glDepthMask(GL_TRUE) );
		GL_WRAP( glDepthFunc(GL_LESS) );		// Write if depth <= depth buffer
		GL_WRAP( glDepthRange(0.0f, 1.0f) );	// The Z coordinate range is [0,1]

		// Enable face culling - tells OpenGL to not draw the faces that
		// cannot be seen
		GL_WRAP( glEnable(GL_CULL_FACE) );
		GL_WRAP( glCullFace(GL_BACK) );
		GL_WRAP( glFrontFace(GL_CCW) );			// Render only the counter-clockwise faces

		// The Clear Color of the window
		GL_WRAP( glClearColor(0.0f, 0.75f, 1.0f, 1.0f) );
	}


	std::string GraphicsSystem::getGLInfo() const
	{
		return	"OpenGL Renderer: " + std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER))) +
				"\nOpenGL version supported " + std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))) +
				"\nGLSL version supported " + std::string(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
	}


	void GraphicsSystem::setViewport(int width, int height)
	{
		GL_WRAP( glViewport(0, 0, width, height) );
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
		GL_WRAP( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
		for (ILayer* layer : mLayers) {
			layer->render();
		}
	}

}
