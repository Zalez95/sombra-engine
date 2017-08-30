#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "PrimitiveTypes.h"

namespace graphics {

	class Shader;


	/**
	 * Program Class, the program class is used to load the GLSL programs
	 * and access to its uniform variables
	 */
	class Program
	{
	private:	// Attributes
		/** The reference of the shader object */
		GLuint mProgramID;

	public:		// Functions
		/** Creates and compiles a OpenGL program from the specified shaders
		 *
		 * @param	shaders a vector with the shaders that compose the GLSL
		 *			Program */
		Program(const std::vector<const Shader*>& shaders);

		/** Class destructor */
		~Program();

		/** Return the location of the given uniform name
		 *
		 * @param	name the name of the uniform variable that we want
		 *			the location
		 * @return	the location of the uniform variable */
		GLuint getUniformLocation(const char* name) const;

		void setUniform(const char* name,	int value) const;
		void setUniform(GLuint location,	int value) const;
		
		void setUniform(const char* name,	float value) const;
		void setUniform(GLuint location,	float value) const;
		
		void setUniform(const char* name,	const RGBColor& color) const;
		void setUniform(GLuint location,	const RGBColor& color) const;

		void setUniform(const char* name,	const RGBAColor& color) const;
		void setUniform(GLuint location,	const RGBAColor& color) const;

		void setUniform(const char* name,	const glm::vec2& vector) const;
		void setUniform(GLuint location,	const glm::vec2& vector) const;
		
		void setUniform(const char* name,	const glm::vec3& vector) const;
		void setUniform(GLuint location,	const glm::vec3& vector) const;
		
		void setUniform(const char* name,	const glm::vec4& vector) const;
		void setUniform(GLuint location,	const glm::vec4& vector) const;
		
		void setUniform(const char* name,	const glm::mat3& matrix) const;
		void setUniform(GLuint location,	const glm::mat3& matrix) const;

		void setUniform(const char* name,	const glm::mat4& matrix) const;
		void setUniform(GLuint location,	const glm::mat4& matrix) const;
		
		/** Uses the current shader object so they can be used as part 
		 * of the current rendering state */
		void enable() const;
		
		/** Resets the current shader object */
		void disable();
	};

}

#endif		// PROGRAM_H
