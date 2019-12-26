#ifndef PROGRAM_3D_H
#define PROGRAM_3D_H

#include <glm/glm.hpp>
#include "../IProgram.h"

namespace se::graphics {

	/**
	 * Program3D class, it's a high level Program used by the Renderer so
	 * it doesn't need to search and set the uniform variables
	 */
	class Program3D : public IProgram
	{
	protected:	// Nested types
		struct TextureUnits {
			static constexpr int kBaseColor = 0;
		};

	public:		// Functions
		/** Class destructor */
		virtual ~Program3D() {};

		/** Sets the uniform variables for the given View matrix
		 *
		 * @param	viewMatrix the matrix that we want to set as the
		 *			View matrix in the shaders */
		void setViewMatrix(const glm::mat4& viewMatrix);

		/** Sets the uniform variables for the given Projection matrix
		 *
		 * @param	projectionMatrix the matrix that we want to set as the
		 *			Projection matrix in the shaders */
		void setProjectionMatrix(const glm::mat4& projectionMatrix);
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

#endif		// PROGRAM_3D_H
