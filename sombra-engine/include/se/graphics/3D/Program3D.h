#ifndef PROGRAM_3D_H
#define PROGRAM_3D_H

#include <glm/glm.hpp>

namespace se::graphics {

	class Program;


	/**
	 * Program3D class, it's a high level Program used by the Renderer so
	 * it doesn't need to search and set the uniform variables
	 */
	class Program3D
	{
	protected:	// Nested types
		struct TextureUnits {
			static constexpr int kBaseColor = 0;
		};

	protected:	// Attributes
		/** The Program of the renderer */
		Program* mProgram;

	public:		// Functions
		/** Creates a new Program3D */
		Program3D() : mProgram(nullptr) {};

		/** Class destructor */
		virtual ~Program3D() {};

		/** Function called for creating the program and initializing all the
		 * needed resources
		 *
		 * @return	true on success, false otherwise */
		virtual bool init();

		/** Function called for deleting the program and cleaning all the
		 * needed resources
		 *
		 * @return	true on success, false otherwise */
		virtual bool end();

		/** Uses the current shader object so they can be used as part
		 * of the current rendering state */
		void enable() const;

		/** Resets the current shader object */
		void disable() const;

		/** Sets the uniform variables fot the given Model matrix
		 *
		 * @param	modelMatrix the matrix that we want to set as the
		 *			Model matrix in the shaders */
		void setModelMatrix(const glm::mat4& modelMatrix) const;

		/** Sets the uniform variables for the given View matrix
		 *
		 * @param	viewMatrix the matrix that we want to set as the
		 *			View matrix in the shaders */
		void setViewMatrix(const glm::mat4& viewMatrix) const;

		/** Sets the uniform variables for the given Projection matrix
		 *
		 * @param	projectionMatrix the matrix that we want to set as the
		 *			Projection matrix in the shaders */
		void setProjectionMatrix(const glm::mat4& projectionMatrix) const;
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

#endif		// PROGRAM_3D_H
