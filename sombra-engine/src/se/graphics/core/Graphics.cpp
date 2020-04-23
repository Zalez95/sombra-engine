#include "se/graphics/core/Graphics.h"
#include "GLWrapper.h"

namespace se::graphics {

	bool Graphics::init()
	{
		glewExperimental = true;
		if (glewInit() != GLEW_OK) {
			SOMBRA_FATAL_LOG << "Failed to initialize GLEW";
			return false;
		}

		// Allow non-aligned textures
		GL_WRAP( glPixelStorei(GL_UNPACK_ALIGNMENT, 1) );

		return true;
	}


	std::string Graphics::getGraphicsInfo()
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


	void Graphics::setViewport(int x, int y, std::size_t width, std::size_t height)
	{
		GL_WRAP( glViewport(x, y, static_cast<GLsizei>(width), static_cast<GLsizei>(height)) );
	}


	void Graphics::setCulling(bool active)
	{
		if (active) {
			GL_WRAP( glEnable(GL_CULL_FACE) );
		}
		else {
			GL_WRAP( glDisable(GL_CULL_FACE) );
		}
	}


	void Graphics::setDepthTest(bool active)
	{
		if (active) {
			GL_WRAP( glEnable(GL_DEPTH_TEST) );
		}
		else {
			GL_WRAP( glDisable(GL_DEPTH_TEST) );
		}
	}


	void Graphics::setBlending(bool active)
	{
		if (active) {
			GL_WRAP( glEnable(GL_BLEND) );
			GL_WRAP( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
		}
		else {
			GL_WRAP( glDisable(GL_BLEND) );
		}
	}


	void Graphics::drawIndexed(PrimitiveType primitive, std::size_t indexCount, TypeId indexType)
	{
		GL_WRAP( glDrawElements(toGLPrimitive(primitive), static_cast<GLsizei>(indexCount), toGLType(indexType), nullptr) );
	}


	void Graphics::drawArrays(PrimitiveType primitive, std::size_t vertexCount)
	{
		GL_WRAP( glDrawArrays(toGLPrimitive(primitive), 0, static_cast<GLsizei>(vertexCount)) );
	}


	void Graphics::drawArraysInstanced(PrimitiveType primitive, std::size_t vertexCount, std::size_t instanceCount)
	{
		GL_WRAP( glDrawArraysInstanced(toGLPrimitive(primitive), 0, static_cast<GLsizei>(vertexCount), static_cast<GLsizei>(instanceCount)) );
	}


	void Graphics::clear(bool color, bool depth, bool stencil)
	{
		GLbitfield mask = 0;
		if (color) {
			mask |= GL_COLOR_BUFFER_BIT;
		}
		if (depth) {
			mask |= GL_DEPTH_BUFFER_BIT;
		}
		if (stencil) {
			mask |= GL_STENCIL_BUFFER_BIT;
		}

		GL_WRAP( glClear(mask) );
	}

}
