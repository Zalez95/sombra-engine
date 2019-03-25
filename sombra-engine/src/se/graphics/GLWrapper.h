#include <GL/glew.h>
#include "se/utils/Log.h"
#include "se/graphics/Constants.h"

#define GL_WRAP(x)					\
	se::graphics::glClearError();	\
	x;								\
	se::graphics::glLogError(#x, LOCATION);


namespace se::graphics {

	static void glClearError()
	{
		while (glGetError() != GL_NO_ERROR);
	}


	static void glLogError(const char* glFunction, const std::string& location)
	{
		GLenum error;
		while ((error = glGetError()) != GL_NO_ERROR) {
			std::string errorName = "";
			switch (error) {
				case GL_INVALID_ENUM:					errorName = "INVALID_ENUM";						break;
				case GL_INVALID_VALUE:					errorName = "INVALID_VALUE";					break;
				case GL_INVALID_OPERATION:				errorName = "INVALID_OPERATION";				break;
				case GL_STACK_OVERFLOW:					errorName = "STACK_OVERFLOW";					break;
				case GL_STACK_UNDERFLOW:				errorName = "STACK_UNDERFLOW";					break;
				case GL_OUT_OF_MEMORY:					errorName = "OUT_OF_MEMORY";					break;
				case GL_INVALID_FRAMEBUFFER_OPERATION:	errorName = "INVALID_FRAMEBUFFER_OPERATION";	break;
			}

			utils::Log::getInstance()(utils::LogLevel::Error) << location
				<< "OpenGL function \"" << glFunction << "\" returned error code " << error
				<< " (" << errorName << "): \"" << glGetString(error) << "\"";
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
