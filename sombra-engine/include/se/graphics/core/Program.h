#ifndef PROGRAM_H
#define PROGRAM_H

#include <cstddef>
#include "Bindable.h"

namespace se::graphics {

	class Shader;


	/**
	 * Program Class, the program class is used to load the GLSL programs
	 * and access to its uniform variables
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

		/** Uses the current shader object so they can be used as part
		 * of the current rendering state */
		void bind() const override;

		/** Resets the current shader object */
		void unbind() const override;
	};

}

#endif		// PROGRAM_H
