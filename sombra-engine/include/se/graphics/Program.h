#ifndef PROGRAM_H
#define PROGRAM_H

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
		template <typename T>
		void setUniform(const char* name, const T& value) const;

		/** Sets the value of the given uniform variable
		 *
		 * @param	location the uniform variable location
		 * @param	value the new value of the variable
		 * @note	if the location is -1, the data will be ignored */
		template <typename T>
		void setUniform(int location, const T& value) const;

		/** Sets the values of the given array of values uniform variable
		 *
		 * @param	name the name of the uniform variable that we want to set
		 * @param	count the number of elements to set
		 * @param	valuePtr a pointer to the vector of values
		 * @note	if the location is -1, the data will be ignored */
		template <typename T>
		void setUniformV(
			const char* name, std::size_t count, const T* valuePtr
		) const;

		/** Sets the values of the given array of values uniform variable
		 *
		 * @param	location the uniform variable location
		 * @param	count the number of elements to set
		 * @param	valuePtr a pointer to the vector of values
		 * @note	if the location is -1, the data will be ignored */
		template <typename T>
		void setUniformV(
			int location, std::size_t count, const T* valuePtr
		) const;

		/** Uses the current shader object so they can be used as part
		 * of the current rendering state */
		void enable() const;

		/** Resets the current shader object */
		void disable() const;
	};

}

#include "Program.hpp"

#endif		// PROGRAM_H
