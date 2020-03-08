#include <GL/glew.h>
#include "se/utils/Log.h"
#include "se/graphics/core/Constants.h"

#define GL_WRAP(x)					\
	se::graphics::glClearError();	\
	x;								\
	se::graphics::glLogError(#x, __FUNCTION__, __LINE__);


namespace se::graphics {

	static void glClearError()
	{
		while (glGetError() != GL_NO_ERROR);
	}


	static void glLogError(const char* glFunction, const char* function, int line)
	{
		GLenum error;
		while ((error = glGetError()) != GL_NO_ERROR) {
			std::string errorTag = "";
			switch (error) {
				case GL_INVALID_ENUM:					errorTag = "INVALID_ENUM";					break;
				case GL_INVALID_VALUE:					errorTag = "INVALID_VALUE";					break;
				case GL_INVALID_OPERATION:				errorTag = "INVALID_OPERATION";				break;
				case GL_STACK_OVERFLOW:					errorTag = "STACK_OVERFLOW";				break;
				case GL_STACK_UNDERFLOW:				errorTag = "STACK_UNDERFLOW";				break;
				case GL_OUT_OF_MEMORY:					errorTag = "OUT_OF_MEMORY";					break;
				case GL_INVALID_FRAMEBUFFER_OPERATION:	errorTag = "INVALID_FRAMEBUFFER_OPERATION";	break;
			}

			const char* errorString = reinterpret_cast<const char*>( glGetString(error) );

			utils::Log::getInstance()(utils::LogLevel::Error) << FORMAT_LOCATION(function, line)
				<< "OpenGL function \"" << glFunction << "\" returned error code " << error
				<< " (" << errorTag << "): \"" << (errorString? errorString : "") << "\"";
		}
	}


	constexpr GLenum toGLType(TypeId type)
	{
		switch (type) {
			case TypeId::Byte:			return GL_BYTE;
			case TypeId::UnsignedByte:	return GL_UNSIGNED_BYTE;
			case TypeId::Short:			return GL_SHORT;
			case TypeId::UnsignedShort:	return GL_UNSIGNED_SHORT;
			case TypeId::Int:			return GL_INT;
			case TypeId::UnsignedInt:	return GL_UNSIGNED_INT;
			case TypeId::Float:			return GL_FLOAT;
			case TypeId::HalfFloat:		return GL_HALF_FLOAT;
			case TypeId::Double:		return GL_DOUBLE;
			default:					return GL_NONE;
		}
	}


	constexpr GLenum toGLShader(ShaderType type)
	{
		switch (type) {
			case ShaderType::Vertex:	return GL_VERTEX_SHADER;
			case ShaderType::Geometry:	return GL_GEOMETRY_SHADER;
			case ShaderType::Fragment:	return GL_FRAGMENT_SHADER;
			default:					return GL_NONE;
		}
	}


	constexpr GLenum toGLColor(ColorFormat format)
	{
		switch (format) {
			case ColorFormat::Red:		return GL_RED;
			case ColorFormat::Green:	return GL_GREEN;
			case ColorFormat::Blue:		return GL_BLUE;
			case ColorFormat::Alpha:	return GL_ALPHA;
			case ColorFormat::RGB:		return GL_RGB;
			case ColorFormat::RGBA:		return GL_RGBA;
			default:					return GL_NONE;
		}
	}


	constexpr int toGLFilter(TextureFilter filter)
	{
		switch (filter) {
			case TextureFilter::Nearest:				return GL_NEAREST;
			case TextureFilter::Linear:					return GL_LINEAR;
			case TextureFilter::NearestMipMapNearest:	return GL_NEAREST_MIPMAP_NEAREST;
			case TextureFilter::LinearMipMapNearest:	return GL_LINEAR_MIPMAP_NEAREST;
			case TextureFilter::NearestMipMapLinear:	return GL_NEAREST_MIPMAP_LINEAR;
			case TextureFilter::LinearMipMapLinear:		return GL_LINEAR_MIPMAP_LINEAR;
			default:									return GL_NONE;
		}
	}


	constexpr int toGLWrap(TextureWrap wrap)
	{
		switch (wrap) {
			case TextureWrap::Repeat:			return GL_REPEAT;
			case TextureWrap::MirroredRepeat:	return GL_MIRRORED_REPEAT;
			case TextureWrap::ClampToEdge:		return GL_CLAMP_TO_EDGE;
			case TextureWrap::ClampToBorder:	return GL_CLAMP_TO_BORDER;
			default:							return GL_NONE;
		}
	}

}
