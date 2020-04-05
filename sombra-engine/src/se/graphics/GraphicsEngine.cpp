#include <stdexcept>
#include <algorithm>
#include "se/graphics/GraphicsEngine.h"
#include "core/GLWrapper.h"

namespace se::graphics {

	GraphicsEngine::GraphicsEngine(const glm::uvec2& viewportSize) : mViewportSize(viewportSize)
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

		// Allow non-aligned textures
		GL_WRAP( glPixelStorei(GL_UNPACK_ALIGNMENT, 1) );
	}


	std::string GraphicsEngine::getGLInfo() const
	{
		GL_WRAP( const char* glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER)) );
		GL_WRAP( const char* glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION)) );
		GL_WRAP( const char* glslVersion = reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)) );

		int maxVertexUniforms = -1;
		GL_WRAP( glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUniforms) );

		int maxGeometryUniforms = -1;
		GL_WRAP( glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_COMPONENTS, &maxGeometryUniforms) );

		int maxFragmentUniforms = -1;
		GL_WRAP( glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &maxFragmentUniforms) );

		int maxTextureUnits = -1;
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);

		return	std::string("OpenGL Renderer: ") + glRenderer + "\n"
				+ "OpenGL version supported: " + glVersion + "\n"
				+ "GLSL version supported: " + glslVersion + "\n"
				+ "Max vertex uniforms: " + std::to_string(maxVertexUniforms) + "\n"
				+ "Max geometry uniforms: " + std::to_string(maxGeometryUniforms) + "\n"
				+ "Max fragment uniforms: " + std::to_string(maxFragmentUniforms) + "\n"
				+ "Max texture units: " + std::to_string(maxTextureUnits) + "\n";
	}


	void GraphicsEngine::setViewport(const glm::uvec2& viewportSize)
	{
		mViewportSize = viewportSize;

		GL_WRAP( glViewport(0, 0, mViewportSize.x, mViewportSize.y) );

		for (ILayer* layer : mLayers) {
			layer->setViewportSize(viewportSize);
		}
	}


	void GraphicsEngine::addLayer(ILayer* layer)
	{
		if (layer) {
			layer->setViewportSize(mViewportSize);

			mLayers.push_back(layer);
		}
	}


	void GraphicsEngine::removeLayer(ILayer* layer)
	{
		mLayers.erase(
			std::remove(mLayers.begin(), mLayers.end(), layer),
			mLayers.end()
		);
	}


	void GraphicsEngine::render()
	{
		GL_WRAP( glClear(GL_DEPTH_BUFFER_BIT) );

		for (ILayer* layer : mLayers) {
			layer->render();
		}
	}

}
