#include <string>
#include <GL/glew.h>
#include "se/utils/Logger.h"

#define GL_WRAP(x)					\
	se::graphics::glClearError();	\
	x;								\
	se::graphics::glLogError(#x);


namespace se::graphics {

	static void glClearError()
	{
		while (glGetError() != GL_NO_ERROR);
	}


	static bool glLogError(const std::string& functionName)
	{
		GLenum error = glGetError();
		while (error != GL_NO_ERROR) {
			utils::Logger::getInstance().write(
				utils::LogLevel::ERROR,
				"OpenGL function \"" + functionName + "\" returned error: "
					+ std::to_string(error)
			);
			return true;
		}

		return false;
	}

}
