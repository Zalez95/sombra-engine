#ifndef PROGRAM_2D_H
#define PROGRAM_2D_H

#include <glm/glm.hpp>

namespace se::graphics {

	class Program;


	/**
	 * Program2D class, it's a high level Program used by the Renderer2D so it
	 * doesn't need to search and set the uniform variables
	 */
	class Program2D
	{
	private:	// Attributes
		/** The Program of the renderer */
		Program* mProgram;

	public:		// Functions
		/** Creates a new Program2D */
		Program2D() : mProgram(nullptr) {};

		/** Function called for initializing all the needed resources
		 *
		 * @return	true on success, false otherwise */
		virtual bool init();

		/** Function called for cleaning all the needed resources */
		virtual void end();

		/** Uses the current shader object so they can be used as part
		 * of the current rendering state */
		void enable() const;

		/** Resets the current shader object */
		void disable() const;

		/** Sets the uniform variables fot the given Model matrix
		 *
		 * @param	modelMatrix the matrix that we want to set as the
		 *			Model matrix in the shaders */
		void setModelMatrix(const glm::mat4& modelMatrix);

		/** Sets the uniform variables for the texture sampler
		 *
		 * @param	unit the id of the texture sampler */
		void setTextureSampler(int unit);
	protected:
		/** Creates the Shaders and the Program that the current class will use
		 * for setting the uniform variables
		 *
		 * @param	vertexShaderPath the path to the vertex shader of the new
		 *			Program
		 * @param	fragmentShaderPath the path to the fragment shader of the
		 *			new Program
		 * @return	true if the shaders were loaded successfully, false
		 *			otherwise */
		virtual bool createProgram(
			const char* vertexShaderPath,
			const char* fragmentShaderPath
		);

		/** Adds the uniform variables to the program */
		virtual bool addUniforms();
	};

}

#endif		// PROGRAM_2D_H
