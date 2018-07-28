#ifndef SHADER_H
#define SHADER_H

namespace fe { namespace graphics {

	/**
	 * Class Shader, it represents a GLSL shader. A shader is one of the
	 * programable stages of the OpenGL pipeline
	 */
	class Shader
	{
	private:	// Attributes
		/** The ID of the Shader */
		unsigned int mShaderID;

	public:		// Functions
		/** Creates and compiles a shader from the specified source code.
		 *
		 * @param	source the GLSL source code of the shader that we want to
		 *			create
		 * @param	shaderType the type of the shader that we want to create
		 *			(Usually GL_VERTEX_SHADER or GL_FRAGMENT_SHADER)
		 * @throw	runtime_error if the Shader couldn't be created */
		Shader(const char* source, unsigned int shaderType);

		/** Class destructor */
		~Shader();

		/** @return the ID of the Shader */
		inline unsigned int getShaderID() const { return mShaderID; };
	};

}}

#endif		// SHADER_H
