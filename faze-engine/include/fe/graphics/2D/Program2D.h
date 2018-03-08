#ifndef PROGRAM_2D_H
#define PROGRAM_2D_H

#include <glm/glm.hpp>

namespace fe { namespace graphics {

	class Program;


	/** Program2D class, it's a high level Program used by the
	 * SceneRenderer so it doesn't need to search and set the uniform
	 * variables */
	class Program2D
	{
	private:	// Attributes
		/** The Program of the renderer */
		Program* mProgram;

		/** Holds the uniform variables location so we don't have to get them
		 * in each render call */
		struct UniformLocations
		{
			unsigned int modelMatrix;
			unsigned int textureSampler;
		} mUniformLocations;

	public:		// Functions
		/** Creates a new Program2D */
		Program2D();

		/** Class destructor */
		~Program2D();

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

		void setTextureSampler(int unit);
	private:
		/** Creates the Shaders and the Program that the current class will use
		 * for setting the uniform variables */
		void initShaders();

		/** Gets the location of all the uniform variables and stores them in
		 * mUniformLocations */
		void initUniformLocations();
	};

}}

#endif		// PROGRAM_2D_H
