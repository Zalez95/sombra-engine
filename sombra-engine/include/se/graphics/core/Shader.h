#ifndef SHADER_H
#define SHADER_H

#include "Constants.h"

namespace se::graphics {

	/**
	 * Class Shader, it represents a GLSL shader. A shader is one of the
	 * programable stages of the OpenGL pipeline
	 */
	class Shader
	{
	private:	// Attributes
		friend class Program;

		/** The id of the Shader */
		unsigned int mShaderId;

	public:		// Functions
		/** Creates and compiles a shader from the specified source code.
		 *
		 * @param	source the GLSL source code of the shader that we want to
		 *			create
		 * @param	shaderType the type of the shader that we want to create
		 * @throw	runtime_error if the Shader couldn't be created */
		Shader(const char* source, ShaderType shaderType);
		Shader(const Shader& other) = delete;
		Shader(Shader&& other);

		/** Class destructor */
		~Shader();

		/** Assignment operator */
		Shader& operator=(const Shader& other) = delete;
		Shader& operator=(Shader&& other);
	};

}

#endif		// SHADER_H
