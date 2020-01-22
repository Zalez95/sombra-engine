#ifndef PROGRAM_SPLATMAP_H
#define PROGRAM_SPLATMAP_H

#include "ProgramLight.h"

namespace se::graphics {

	struct SplatmapMaterial;


	/**
	 * ProgramSplatmap class, it's a high level Program used by the Renderer so
	 * it doesn't need to search and set the uniform variables
	 */
	class ProgramSplatmap : public ProgramLight
	{
	protected:	// Nested types
		struct TextureUnits {
			static constexpr int kBaseColor0			= 0;
			static constexpr int kMetallicRoughness0	= 1;
			static constexpr int kNormal0				= 2;
			static constexpr int kSplatmap				= 13;
		};

	public:		// Functions
		/** Class destructor */
		virtual ~ProgramSplatmap() = default;

		/** Sets the uniforms and other properties needed for rendering with the
		 * given material
		 *
		 * @param	material the material with the data that we want to set */
		void setMaterial(const SplatmapMaterial& material);
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

#endif		// PROGRAM_SPLATMAP_H
