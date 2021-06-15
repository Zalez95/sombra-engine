#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include "../../graphics/core/Program.h"
#include "../graphics/Material.h"
#include "../graphics/RenderableShader.h"
#include "Result.h"

namespace se::app {

	/** Class ShaderLoader, it used for loading Programs and adding Bindables
	 * to steps */
	class ShaderLoader
	{
	private:	// Nested types
		using ProgramRef = Repository::ResourceRef<graphics::Program>;
		using StepRef = Repository::ResourceRef<RenderableShaderStep>;
		using TextureRef = Repository::ResourceRef<graphics::Texture>;

	public:		// Functions
		/** Creates the bindables of the given material and adds them to the
		 * step
		 *
		 * @param	step the RenderableShaderStep to add the Bindables
		 * @param	material the material
		 * @param	program the Program of the Step */
		static void addMaterialBindables(
			StepRef step, const Material& material, const ProgramRef& program
		);

		/** Reads the bindables data and adds it to the given Material
		 *
		 * @param	step the RenderableShaderStep to read its Bindables
		 * @param	material the Material (return parameter) */
		static void readMaterialBindables(
			const StepRef& step, Material& material
		);

		/** Creates the bindables of the given splatmap material and adds them
		 * to the strp
		 *
		 * @param	step the RenderableShaderStep to add the Bindables
		 * @param	material the SplatmapMaterial
		 * @param	program the Program of the Step */
		static void addSplatmapMaterialBindables(
			StepRef step, const SplatmapMaterial& material,
			const ProgramRef& program
		);

		/** Reads the bindables data and adds it to the given SplatmapMaterial
		 *
		 * @param	step the RenderableShaderStep to read its Bindables
		 * @param	material the SplatmapMaterial (return parameter) */
		static void readSplatmapMaterialBindables(
			const StepRef& step, SplatmapMaterial& material
		);

		/** Creates the bindables of the given height map and adds them to the
		 * step
		 *
		 * @param	step the RenderableShaderStep to add the Bindables
		 * @param	heightMap the heightmap to add
		 * @param	size the size in the X and Z axis of the height map
		 * @param	maxHeight the maximum height of the height map
		 * @param	program the Program of the Step */
		static void addHeightMapBindables(
			StepRef step,
			const TextureRef& heightMap, float size, float maxHeight,
			const ProgramRef& program
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
			std::shared_ptr<graphics::Program>& program
		);
	};

}

#endif		// SHADER_LOADER_H
