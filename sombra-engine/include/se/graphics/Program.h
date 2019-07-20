#ifndef PROGRAM_H
#define PROGRAM_H

#include <glm/glm.hpp>
#include "PrimitiveTypes.h"

namespace se::graphics {

	class Shader;


	/**
	 * Program Class, the program class is used to load the GLSL programs
	 * and access to its uniform variables
	 */
	class Program
	{
	private:	// Attributes
		/** The id of the shader object */
		unsigned int mProgramId;

	public:		// Functions
		/** Creates and links a OpenGL Program from the specified shaders
		 *
		 * @param	shaders the shaders that creates the GLSL Program
		 * @param	shaderCount the number of shaders
		 * @throw	runtime_error if the Program couldn't be created */
		Program(const Shader* shaders[], std::size_t shaderCount);
		Program(const Program& other) = delete;
		Program(Program&& other);

		/** Class destructor */
		~Program();

		/** Assignment operator */
		Program& operator=(const Program& other) = delete;
		Program& operator=(Program&& other);

		/** Return the location of the given uniform name
		 *
		 * @param	name the name of the uniform variable that we want
		 *			the location
		 * @return	the location of the uniform variable, -1 if the uniform
		 *			name wasn't found */
		int getUniformLocation(const char* name) const;

		/** Sets the value of the given uniform variable
		 *
		 * @param	name the name of the uniform variable that we want to set
		 * @param	value the new value of the variable
		 * @note	if the name wasn't found, the data will be ignored */
		void setUniform(const char* name, int value) const;

		/** Sets the value of the given uniform variable
		 *
		 * @param	location the uniform variable location
		 * @param	value the new value of the variable
		 * @note	if the location is -1, the data will be ignored */
		void setUniform(int location, int value) const;

		/** Sets the value of the given uniform variable
		 *
		 * @param	name the name of the uniform variable that we want to set
		 * @param	value the new value of the variable
		 * @note	if the name wasn't found, the data will be ignored */
		void setUniform(const char* name, unsigned int value) const;

		/** Sets the value of the given uniform variable
		 *
		 * @param	location the uniform variable location
		 * @param	value the new value of the variable
		 * @note	if the location is -1, the data will be ignored */
		void setUniform(int location, unsigned int value) const;

		/** Sets the value of the given uniform variable
		 *
		 * @param	name the name of the uniform variable that we want to set
		 * @param	value the new value of the variable
		 * @note	if the name wasn't found, the data will be ignored */
		void setUniform(const char* name, float value) const;

		/** Sets the value of the given uniform variable
		 *
		 * @param	location the uniform variable location
		 * @param	value the new value of the variable
		 * @note	if the location is -1, the data will be ignored */
		void setUniform(int location, float value) const;

		/** Sets the value of the given vector uniform variable
		 *
		 * @param	name the name of the uniform variable that we want to set
		 * @param	vector the new value of the variable
		 * @note	if the name wasn't found, the data will be ignored */
		void setUniform(const char* name, const glm::vec2& vector) const;

		/** Sets the value of the given vector uniform variable
		 *
		 * @param	location the uniform variable location
		 * @param	vector the new value of the variable
		 * @note	if the location is -1, the data will be ignored */
		void setUniform(int location, const glm::vec2& vector) const;

		/** Sets the value of the given vector uniform variable
		 *
		 * @param	name the name of the uniform variable that we want to set
		 * @param	vector the new value of the variable
		 * @note	if the name wasn't found, the data will be ignored */
		void setUniform(const char* name, const glm::vec3& vector) const;

		/** Sets the value of the given vector uniform variable
		 *
		 * @param	location the uniform variable location
		 * @param	vector the new value of the variable
		 * @note	if the location is -1, the data will be ignored */
		void setUniform(int location, const glm::vec3& vector) const;

		/** Sets the value of the given vector uniform variable
		 *
		 * @param	name the name of the uniform variable that we want to set
		 * @param	vector the new value of the variable
		 * @note	if the name wasn't found, the data will be ignored */
		void setUniform(const char* name, const glm::vec4& vector) const;

		/** Sets the value of the given vector uniform variable
		 *
		 * @param	location the uniform variable location
		 * @param	vector the new value of the variable
		 * @note	if the location is -1, the data will be ignored */
		void setUniform(int location, const glm::vec4& vector) const;

		/** Sets the value of the given matrix uniform variable
		 *
		 * @param	name the name of the uniform variable that we want to set
		 * @param	matrix the new value of the variable
		 * @note	if the name wasn't found, the data will be ignored */
		void setUniform(const char* name, const glm::mat3& matrix) const;

		/** Sets the value of the given matrix uniform variable
		 *
		 * @param	location the uniform variable location
		 * @param	matrix the new value of the variable
		 * @note	if the location is -1, the data will be ignored */
		void setUniform(int location, const glm::mat3& matrix) const;

		/** Sets the value of the given matrix uniform variable
		 *
		 * @param	name the name of the uniform variable that we want to set
		 * @param	matrix the new value of the variable
		 * @note	if the name wasn't found, the data will be ignored */
		void setUniform(const char* name, const glm::mat4& matrix) const;

		/** Sets the value of the given matrix uniform variable
		 *
		 * @param	location the uniform variable location
		 * @param	matrix the new value of the variable
		 * @note	if the location is -1, the data will be ignored */
		void setUniform(int location, const glm::mat4& matrix) const;

		/** Uses the current shader object so they can be used as part
		 * of the current rendering state */
		void enable() const;

		/** Resets the current shader object */
		void disable() const;
	};

}

#endif		// PROGRAM_H
