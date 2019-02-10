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


	static bool glLogError(const char* glFunction, const std::string& location)
	{
		GLenum error = glGetError();
		while (error != GL_NO_ERROR) {
			utils::Log::getInstance()(utils::LogLevel::Error) << location
				<< "OpenGL function \"" << glFunction << "\" returned error code " << error
				<< ": \"" << glGetString(error) << "\"";
			return true;
		}

		return false;
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

}
