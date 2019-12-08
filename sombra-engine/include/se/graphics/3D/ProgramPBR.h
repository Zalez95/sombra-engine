#ifndef PROGRAM_PBR_H
#define PROGRAM_PBR_H

#include <vector>
#include <glm/glm.hpp>
#include "Program3D.h"

namespace se::graphics {

	struct Material;
	struct ILight;


	/**
	 * ProgramPBR class, it's a high level Program used by the Renderer so
	 * it doesn't need to search and set the uniform variables
	 */
	class ProgramPBR : public Program3D
	{
	protected:	// Nested types
		struct TextureUnits {
			static constexpr int kBaseColor			= 0;
			static constexpr int kMetallicRoughness	= 1;
			static constexpr int kNormal			= 2;
			static constexpr int kOcclusion			= 3;
			static constexpr int kEmissive			= 4;
		};

	protected:	// Attributes
		/** The maximum number of point lights in the program */
		static constexpr int kMaxPointLights = 4;

	public:		// Functions
		/** Class destructor */
		virtual ~ProgramPBR() {};

		/** Sets the uniform variables fot the given Model matrix
		 *
		 * @param	modelMatrix the matrix that we want to set as the
		 *			Model matrix in the shaders */
		void setModelMatrix(const glm::mat4& modelMatrix) const;

		/** Sets the uniforms and other properties needed for rendering with the
		 * given material
		 *
		 * @param	material the material with the data that we want to set */
		void setMaterial(const Material& material) const;

		/** Clears the properties setted for rendering with the given material
		 *
		 * @param	material the material with the data that we want to unset */
		void unsetMaterial(const Material& material) const;

		/** Sets the uniform variables for the given ILights
		 *
		 * @param	lights a vector of pointers to the ILights with the data
		 *			that we want to set as uniform variables in the shaders
		 * @note	the maximum number of Lights is MAX_LIGHTS, so if
		 *			there are more lights in the given vector only the first
		 *			lights of the vector will be submited */
		void setLights(const std::vector<const ILight*>& lights) const;
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

#endif		// PROGRAM_PBR_H
