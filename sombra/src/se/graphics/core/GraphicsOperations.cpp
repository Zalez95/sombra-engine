#include "se/graphics/core/GraphicsOperations.h"
#include "GLWrapper.h"

namespace se::graphics {

	bool GraphicsOperations::init()
	{
		glewExperimental = true;
		if (glewInit() != GLEW_OK) {
			SOMBRA_FATAL_LOG << "Failed to initialize GLEW";
			return false;
		}

		if (!glewIsSupported("GL_VERSION_3_3")) {
			SOMBRA_FATAL_LOG << "OpenGL 3.3 is not supported";
			return false;
		}

		/*if (!glewIsSupported("GL_ARB_shader_image_load_store") && !glewIsSupported("GL_EXT_shader_image_load_store")) {
			SOMBRA_FATAL_LOG << "OpenGL image_load_store extension is not supported";
			return false;
		}*/

		// Allow non-aligned textures
		GL_WRAP( glPixelStorei(GL_UNPACK_ALIGNMENT, 1) );

		// Enable interpolation between cubemap faces
		GL_WRAP( glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS) );

		// Set the clear color
		GL_WRAP( glClearColor(0.0f, 0.0f, 0.0f, 1.0f) );

		// Set blending mode
		GL_WRAP( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );

		return true;
	}


	std::string GraphicsOperations::getGraphicsInfo()
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
		GL_WRAP( glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits) );

		int maxTextureSize = -1;
		GL_WRAP( glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize) );

		int max3DTextureSize = -1;
		GL_WRAP( glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &max3DTextureSize) );

		return	std::string("OpenGL Renderer: ") + glRenderer + "\n"
				+ "OpenGL version supported: " + glVersion + "\n"
				+ "GLSL version supported: " + glslVersion + "\n"
				+ "Max vertex uniforms: " + std::to_string(maxVertexUniforms) + "\n"
				+ "Max geometry uniforms: " + std::to_string(maxGeometryUniforms) + "\n"
				+ "Max fragment uniforms: " + std::to_string(maxFragmentUniforms) + "\n"
				+ "Max texture units: " + std::to_string(maxTextureUnits) + "\n"
				+ "Max texture size: " + std::to_string(maxTextureSize) + "\n"
				+ "Max 3D texture size: " + std::to_string(max3DTextureSize) + "\n";
	}


	void GraphicsOperations::setViewport(int x, int y, std::size_t width, std::size_t height)
	{
		GL_WRAP( glViewport(x, y, static_cast<GLsizei>(width), static_cast<GLsizei>(height)) );
	}


	void GraphicsOperations::getViewport(int& x, int& y, std::size_t& width, std::size_t& height)
	{
		int params[4];
		GL_WRAP( glGetIntegerv( GL_VIEWPORT, params ) );

		x = params[0];
		y = params[1];
		width = params[2];
		height = params[3];
	}


	void GraphicsOperations::drawArrays(PrimitiveType primitive, std::size_t vertexCount)
	{
		GL_WRAP( glDrawArrays(toGLPrimitive(primitive), 0, static_cast<GLsizei>(vertexCount)) );
	}


	void GraphicsOperations::drawIndexed(PrimitiveType primitive, std::size_t indexCount, TypeId indexType, std::size_t offset)
	{
		GL_WRAP( glDrawElements(toGLPrimitive(primitive), static_cast<GLsizei>(indexCount), toGLType(indexType), reinterpret_cast<const void*>(offset)) );
	}


	void GraphicsOperations::drawArraysInstanced(PrimitiveType primitive, std::size_t vertexCount, std::size_t instanceCount)
	{
		GL_WRAP( glDrawArraysInstanced(
			toGLPrimitive(primitive), 0, static_cast<GLsizei>(vertexCount),
			static_cast<GLsizei>(instanceCount)
		) );
	}


	void GraphicsOperations::drawIndexedInstanced(PrimitiveType primitive, std::size_t indexCount, TypeId indexType, std::size_t offset, std::size_t instanceCount)
	{
		GL_WRAP( glDrawElementsInstanced(
			toGLPrimitive(primitive), static_cast<GLsizei>(indexCount), toGLType(indexType), reinterpret_cast<const void*>(offset),
			static_cast<GLsizei>(instanceCount)
		) );
	}


	void GraphicsOperations::clear(const FrameBufferMask::Mask& mask)
	{
		GL_WRAP( glClear(toGLFrameBufferMask(mask)) );
	}


	void GraphicsOperations::setOperation(Operation operation, bool active)
	{
		if (active) {
			GL_WRAP( glEnable(toGLOperation(operation)) );
		}
		else {
			GL_WRAP( glDisable(toGLOperation(operation)) );
		}
	}


	bool GraphicsOperations::hasOperation(Operation operation)
	{
		GL_WRAP( bool enabled = glIsEnabled(toGLOperation(operation)) );
		return enabled;
	}


	void GraphicsOperations::setCullingMode(FaceMode mode)
	{
		GL_WRAP( glCullFace(toGLFaceMode(mode)) );
	}


	void GraphicsOperations::setScissorBox(int x, int y, std::size_t width, std::size_t height)
	{
		GL_WRAP( glScissor(x, y, static_cast<GLsizei>(width), static_cast<GLsizei>(height)) );
	}


	void GraphicsOperations::setColorMask(bool r, bool g, bool b, bool a)
	{
		GL_WRAP( glColorMask(r, g, b, a) );
	}


	void GraphicsOperations::setDepthMask(bool active)
	{
		GL_WRAP( glDepthMask(active) );
	}


	bool GraphicsOperations::hasDepthMask()
	{
		GLint ret;
		GL_WRAP( glGetIntegerv(GL_DEPTH_WRITEMASK, &ret) );
		return ret;
	}


	void GraphicsOperations::imageMemoryBarrier()
	{
		GL_WRAP( glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT) );
	}


	void SetOperation::bind() const
	{
		mLastActive = GraphicsOperations::hasOperation(mOperation);
		if (mActive != mLastActive) {
			GraphicsOperations::setOperation(mOperation, mActive);
		}
	}


	void SetOperation::unbind() const
	{
		if (mActive != mLastActive) {
			GraphicsOperations::setOperation(mOperation, mLastActive);
		}
	}


	void SetDepthMask::bind() const
	{
		mLastActive = GraphicsOperations::hasDepthMask();
		if (mActive != mLastActive) {
			GraphicsOperations::setDepthMask(mActive);
		}
	}


	void SetDepthMask::unbind() const
	{
		if (mActive != mLastActive) {
			GraphicsOperations::setDepthMask(mLastActive);
		}
	}

}
