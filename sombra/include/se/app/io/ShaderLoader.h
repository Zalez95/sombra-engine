#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include "../../graphics/Pass.h"
#include "../../graphics/core/Program.h"
#include "../graphics/Material.h"
#include "Result.h"

namespace se::app {

	/** Class ShaderLoader, it used for loading Programs and adding Bindables
	 * to passes */
	class ShaderLoader
	{
	private:	// Nested types
		using ProgramSPtr = std::shared_ptr<graphics::Program>;
		using PassSPtr = std::shared_ptr<graphics::Pass>;
		using TextureSPtr = std::shared_ptr<graphics::Texture>;

	public:		// Functions
		/** Creates the bindables of the given material and adds them to the
		 * pass
		 *
		 * @param	pass the pass to add the Bindables
		 * @param	material the material
		 * @param	program the Program of the Pass */
		static void addMaterialBindables(
			PassSPtr pass, const Material& material,
			const ProgramSPtr program
		);

		/** Reads the bindables data and adds it to the given Material
		 *
		 * @param	pass the pass to read its Bindables
		 * @param	material the Material (return parameter) */
		static void readMaterialBindables(
			const PassSPtr pass, Material& material
		);

		/** Creates the bindables of the given splatmap material and adds them
		 * to the* pass
		 *
		 * @param	pass the pass to add the Bindables
		 * @param	material the SplatmapMaterial
		 * @param	program the Program of the Pass */
		static void addSplatmapMaterialBindables(
			PassSPtr pass, const SplatmapMaterial& material,
			const ProgramSPtr program
		);

		/** Reads the bindables data and adds it to the given SplatmapMaterial
		 *
		 * @param	pass the pass to read its Bindables
		 * @param	material the SplatmapMaterial (return parameter) */
		static void readSplatmapMaterialBindables(
			const PassSPtr pass, SplatmapMaterial& material
		);

		/** Creates the bindables of the given height map and adds them to the
		 * pass
		 *
		 * @param	pass the pass to add the Bindables
		 * @param	heightMap the heightmap to add
		 * @param	size the size in the X and Z axis of the height map
		 * @param	maxHeight the maximum height of the height map
		 * @param	program the Program of the Pass */
		static void addHeightMapBindables(
			PassSPtr pass,
			TextureSPtr heightMap, float size, float maxHeight,
			const ProgramSPtr program
		);

		/** Creates a new program from the given files
		 * @param	vertexShaderPath the path to the vertex shader file
		 * @param	geometryShaderPath the path to the geometry shader file
		 * @param	fragmentShaderPath the path to the fragment shader file
		 * @param	program a pointer to the new program (return parameter)
		 * @return	a Result object with the result of the operation
		 * @note	if one of the path is null it will be ignored */
		static Result createProgram(
			const char* vertexShaderPath,
			const char* geometryShaderPath,
			const char* fragmentShaderPath,
			ProgramSPtr& program
		);
	};

}

#endif		// SHADER_LOADER_H
