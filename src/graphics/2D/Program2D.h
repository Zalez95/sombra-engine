#ifndef PROGRAM_3D_H
#define PROGRAM_3D_H

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace graphics {

	class Program;


	/** Program3D class, it's a high level Program used by the
	 * SceneRenderer so it doesn't need to search and set the uniform
	 * variables */
	class Program3D
	{
	private:	// Nested types
		/** The maximum number of point lights in the program */
		static const unsigned int MAX_POINT_LIGHTS = 4;

		/** Struct UniformLocations, it holds the uniform variables location
		 * so we don't have to get them in each render call */
		struct UniformLocations
		{
			GLuint mModelViewMatrix;
		};

	private:	// Attributes
		/** The Program of the renderer */
		Program* mProgram;

		/** The locations of uniform variables in the shader */
		UniformLocations mUniformLocations;

	public:		// Functions
		/** Creates a new Program3D */
		Program3D();

		/** Class destructor */
		~Program3D();

		/** Uses the current shader object so they can be used as part
		 * of the current rendering state */
		void enable() const;

		/** Resets the current shader object */
		void disable() const;

		/** Sets the uniform variables fot the given ModelView matrix
		 * 
		 * @param	modelViewMatrix the matrix that we want to set as the
		 *			ModelView matrix in the shaders */
		void setModelViewMatrix(const glm::mat4& modelViewMatrix);
	private:
		/** Creates the Shaders and the Program that the current class will use
		 * for setting the uniform variables */
		void initShaders();

		/** Gets the location of all the uniform variables and stores them in
		 * mUniformLocations */
		void initUniformLocations();
	};

}

#endif		// PROGRAM_3D_H
