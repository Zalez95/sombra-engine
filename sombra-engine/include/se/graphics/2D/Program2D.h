#ifndef PROGRAM_2D_H
#define PROGRAM_2D_H

#include <glm/glm.hpp>
#include "../IProgram.h"

namespace se::graphics {

	/**
	 * Program2D class, it's a high level Program used by the Renderer2D so it
	 * doesn't need to search and set the uniform variables
	 */
	class Program2D : public IProgram
	{
	public:		// Functions
		/** Class destructor */
		virtual ~Program2D() {};

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
		 * @return	true if the shaders were loaded successfully, false
		 *			otherwise */
		virtual bool createProgram() override;

		/** Adds the uniform variables to the program
		 *
		 * @return	true if the uniform variables were found, false otherwise */
		virtual bool addUniforms() override;
	};

}

#endif		// PROGRAM_2D_H
