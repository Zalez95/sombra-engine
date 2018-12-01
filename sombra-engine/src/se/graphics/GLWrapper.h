#include <GL/glew.h>
#include "se/utils/Log.h"

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

}
