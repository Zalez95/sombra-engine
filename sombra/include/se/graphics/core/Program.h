#ifndef PROGRAM_H
#define PROGRAM_H

#include <cstddef>
#include "Bindable.h"

namespace se::graphics {

	class Shader;


	/**
	 * Program Class, it's used for loading the GLSL programs and accessing to
	 * their uniform variables
	 */
	class Program : public Bindable
	{
	private:	// Attributes
		friend class IUniformVariable;
		friend class UniformBlock;

		/** The id of the shader object */
		unsigned int mProgramId;

	public:		// Functions
		/** Creates and links a OpenGL Program from the specified shaders
		 *
		 * @param	shaders the shaders that creates the GLSL Program
		 * @param	shaderCount the number of shaders
		 * @throw	runtime_error if the Program couldn't be created */
		Program(const Shader* const* shaders, std::size_t shaderCount);
		Program(const Program& other) = delete;
		Program(Program&& other);

		/** Class destructor */
		~Program();

		/** Assignment operator */
		Program& operator=(const Program& other) = delete;
		Program& operator=(Program&& other);

		/** Returns the index of the requested attribute
		 *
		 * @param	name the name of the Attribute
		 * @return	the index of the requested attribute */
		unsigned int getAttributeLocation(const char* name) const;

		/** Uses the current shader object so they can be used as part
		 * of the current rendering state */
		void bind() const override;

		/** Resets the current shader object */
		void unbind() const override;
	};

}

#endif		// PROGRAM_H
