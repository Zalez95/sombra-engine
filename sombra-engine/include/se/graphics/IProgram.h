#ifndef I_PROGRAM_H
#define I_PROGRAM_H

#include <glm/glm.hpp>

namespace se::graphics {

	class Program;


	/**
	 * IProgram class, it's a high level Program used to set Uniform Variables
	 */
	class IProgram
	{
	protected:	// Attributes
		/** The actual Program */
		Program* mProgram;

	public:		// Functions
		/** Creates a new IProgram */
		IProgram() : mProgram(nullptr) {};

		/** Class destructor */
		virtual ~IProgram() = default;

		/** Function called for initializing all the needed resources
		 *
		 * @return	true on success, false otherwise */
		virtual bool init();

		/** Function called for cleaning all the needed resources */
		virtual void end();

		/** Uses the current shader object so they can be used as part
		 * of the current rendering state */
		virtual void enable() const;

		/** Resets the current shader object */
		virtual void disable() const;
	protected:
		/** Creates the Shaders and the Program that the current class will use
		 * for setting the uniform variables
		 *
		 * @return	true if the shaders were loaded successfully, false
		 *			otherwise */
		virtual bool createProgram() = 0;

		/** Adds the uniform variables to the program
		 *
		 * @return	true if the uniform variables were found, false otherwise */
		virtual bool addUniforms() = 0;
	};

}

#endif		// I_PROGRAM_H
