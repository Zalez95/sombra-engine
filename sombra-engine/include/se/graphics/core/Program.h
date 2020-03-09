#ifndef PROGRAM_H
#define PROGRAM_H

#include <string>
#include <vector>

namespace se::graphics {

	class Shader;


	/**
	 * Program Class, the program class is used to load the GLSL programs
	 * and access to its uniform variables
	 */
	class Program
	{
	private:	// Nested types
		/** Maps an uniform name with its location */
		struct NameLocation
		{
			std::string name;	///< The name of the uniform
			int location;		///< The location of the uniform
		};

	private:	// Attributes
		/** The id of the shader object */
		unsigned int mProgramId;

		/** Maps the uniform variable names with their respective locations */
		std::vector<NameLocation> mUniformLocations;

		/** Maps the uniform block names with their respective locations */
		std::vector<NameLocation> mUniformBlocks;

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

		/** Adds the given uniform variable to the program, so it can be setted
		 * later with the @see setUniform and @see setUniformV functions
		 *
		 * @param	name the name of the uniform variable that we want to add
		 * @return	true if the uniform variable was found in the program
		 *			shaders, false otherwise */
		bool addUniform(const char* name);

		/** Adds the given uniform block to the program, so it can be setted
		 * later with the @see setUniformBlock function
		 *
		 * @param	name the name of the uniform block that we want to set
		 * @return	true if the uniform block was found in the program shaders,
		 *			false otherwise */
		bool addUniformBlock(const char* name);

		/** Sets the value of the given uniform variable
		 *
		 * @param	name the name of the uniform variable that we want to set
		 * @param	value the new value of the variable
		 * @note	if the name wasn't found, the data will be ignored */
		template <typename T>
		void setUniform(const char* name, const T& value) const;

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

		/** Binds the given uniform block index to the given program's uniform
		 * block name
		 *
		 * @param	name the name of the uniform block that we want to set
		 * @param	blockIndex the index of the uniform block to set
		 * @note	if the name wasn't found, the index won't be set */
		void setUniformBlock(const char* name, unsigned int blockIndex) const;

		/** Uses the current shader object so they can be used as part
		 * of the current rendering state */
		void enable() const;

		/** Resets the current shader object */
		void disable() const;
	private:
		/** Compares the given NameLocation with the given name
		 *
		 * @param	nameLocation the NameLocation to compare
		 * @param	name the name to compare with
		 * @return	true if the name of @see nameLocation is smaller than
		 *			@see name, false otherwise */
		static bool compare(const NameLocation& nameLocation, const char* name);

		/** Sets the value of the given uniform variable
		 *
		 * @param	location the uniform variable location
		 * @param	value the new value of the variable
		 * @note	if the location is -1, the data will be ignored */
		template <typename T>
		void setUniform(int location, const T& value) const;

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
	};

}

#include "Program.hpp"

#endif		// PROGRAM_H
