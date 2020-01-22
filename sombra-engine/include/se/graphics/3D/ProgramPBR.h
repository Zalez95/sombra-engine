#ifndef PROGRAM_PBR_H
#define PROGRAM_PBR_H

#include "ProgramLight.h"

namespace se::graphics {

	struct Material;


	/**
	 * ProgramPBR class, it's a high level Program used by the Renderer so
	 * it doesn't need to search and set the uniform variables
	 */
	class ProgramPBR : public ProgramLight
	{
	protected:	// Nested types
		struct TextureUnits {
			static constexpr int kBaseColor			= 0;
			static constexpr int kMetallicRoughness	= 1;
			static constexpr int kNormal			= 2;
			static constexpr int kOcclusion			= 3;
			static constexpr int kEmissive			= 4;
		};

	public:		// Functions
		/** Class destructor */
		virtual ~ProgramPBR() = default;

		/** Sets the uniforms and other properties needed for rendering with the
		 * given material
		 *
		 * @param	material the material with the data that we want to set */
		void setMaterial(const Material& material);

		/** Clears the properties setted for rendering with the given material
		 *
		 * @param	material the material with the data that we want to unset */
		void unsetMaterial(const Material& material);
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
