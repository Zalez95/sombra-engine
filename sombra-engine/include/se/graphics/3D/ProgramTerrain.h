#ifndef PROGRAM_TERRAIN_H
#define PROGRAM_TERRAIN_H

#include <vector>
#include <glm/glm.hpp>
#include "ProgramPBR.h"

namespace se::graphics {

	class Texture;


	/**
	 * ProgramTerrain class, it's a high level Program used by the Renderer so
	 * it doesn't need to search and set the uniform variables
	 */
	class ProgramTerrain : public ProgramPBR
	{
	protected:	// Nested types
		struct TextureUnits {
			static constexpr int kHeightMap			= 5;
		};

	public:		// Functions
		/** Class destructor */
		virtual ~ProgramTerrain() = default;

		/** Sets the size of the terrain in the XZ plane
		 *
		 * @param	xzSize the new size of the terrain in the XZ plane */
		void setXZSize(float xzSize);

		/** Sets the maximum height of the terrain in the Y axis
		 *
		 * @param	maxHeight the new maximum height of the terrain in the Y
		 *			axis */
		void setMaxHeight(float maxHeight);

		/** Sets the height map texture
		 *
		 * @param	heightMap the texture to use as height map */
		void setHeightMap(const Texture& heightMap);
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

#endif		// PROGRAM_TERRAIN_H
