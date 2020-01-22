#ifndef PROGRAM_LIGHT_H
#define PROGRAM_LIGHT_H

#include <vector>
#include <glm/glm.hpp>
#include "Program3D.h"
#include "../core/UniformBuffer.h"

namespace se::graphics {

	struct ILight;


	/**
	 * ProgramLight class, it's a high level Program used by the Renderer so
	 * it doesn't need to search and set the uniform variables
	 */
	class ProgramLight : public Program3D
	{
	protected:	// Nested types
		struct UniformBlockIndices
		{
			static constexpr int kPointLights = 0;
		};

	protected:	// Attributes
		/** The maximum number of point lights in the program */
		static constexpr unsigned int kMaxPointLights = 4;

		/** The Uniform Buffer used to set the PointLights */
		UniformBuffer mPointLightsUBO;

	public:		// Functions
		/** Class destructor */
		virtual ~ProgramLight() = default;

		/** Sets the uniform variables fot the given Model matrix
		 *
		 * @param	modelMatrix the matrix that we want to set as the
		 *			Model matrix in the shaders */
		void setModelMatrix(const glm::mat4& modelMatrix);

		/** Sets the uniform variables for the given ILights
		 *
		 * @param	lights a vector of pointers to the ILights with the data
		 *			that we want to set as uniform variables in the shaders
		 * @note	the maximum number of Lights is MAX_LIGHTS, so if
		 *			there are more lights in the given vector only the first
		 *			lights of the vector will be submited */
		void setLights(const std::vector<const ILight*>& lights);
	protected:
		/** Adds the uniform variables to the program
		 *
		 * @return	true if the uniform variables were found, false otherwise */
		virtual bool addUniforms() override;
	};

}

#endif		// PROGRAM_LIGHT_H
