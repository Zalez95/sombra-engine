#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include "../graphics/Material.h"
#include "../graphics/RenderableShader.h"
#include "Result.h"

namespace se::app {

	/** Class ShaderLoader, it used for loading Programs and adding Bindables
	 * to steps */
	class ShaderLoader
	{
	private:	// Nested types
		using ProgramResource = Repository::ResourceRef<ProgramRef>;
		using TextureResource = Repository::ResourceRef<TextureRef>;
		using StepResource = Repository::ResourceRef<RenderableShaderStep>;

	public:		// Functions
		/** Creates the bindables of the given material and adds them to the
		 * step
		 *
		 * @param	step the RenderableShaderStep to add the Bindables
		 * @param	material the material
		 * @param	program the Program of the Step
		 * @note	the Context of the Program will be the one used for
		 *			creating the Bindables */
		static void addMaterialBindables(
			StepResource step,
			const Material& material, const ProgramResource& program
		);

		/** Reads the bindables data and adds it to the given Material
		 *
		 * @param	step the RenderableShaderStep to read its Bindables
		 * @param	material the Material (return parameter) */
		static void readMaterialBindables(
			const StepResource& step, Material& material
		);

		/** Creates the bindables of the given splatmap material and adds them
		 * to the Step
		 *
		 * @param	step the RenderableShaderStep to add the Bindables
		 * @param	material the SplatmapMaterial
		 * @param	program the Program of the Step
		 * @note	the Context of the Program will be the one used for
		 *			creating the Bindables */
		static void addSplatmapMaterialBindables(
			StepResource step, const SplatmapMaterial& material,
			const ProgramResource& program
		);

		/** Reads the bindables data and adds it to the given SplatmapMaterial
		 *
		 * @param	step the RenderableShaderStep to read its Bindables
		 * @param	material the SplatmapMaterial (return parameter) */
		static void readSplatmapMaterialBindables(
			const StepResource& step, SplatmapMaterial& material
		);

		/** Creates the bindables of the given height map and adds them to the
		 * step
		 *
		 * @param	step the RenderableShaderStep to add the Bindables
		 * @param	heightMap the heightmap to add
		 * @param	normalMap the normalmap to add
		 * @param	size the size in the X and Z axis of the height map
		 * @param	maxHeight the maximum height of the height map
		 * @param	program the Program of the Step
		 * @note	the Context of the Program will be the one used for
		 *			creating the Bindables */
		static void addHeightMapBindables(
			StepResource step,
			const TextureResource& heightMap, const TextureResource& normalMap,
			float size, float maxHeight, const ProgramResource& program
		);

		/** Creates a new program from the given files
		 * @param	vertexShaderPath the path to the vertex shader file
		 * @param	geometryShaderPath the path to the geometry shader file
		 * @param	fragmentShaderPath the path to the fragment shader file
		 * @param	context the Context used for creating the Program
		 * @param	program a reference to the new program (return parameter)
		 * @return	a Result object with the result of the operation
		 * @note	if one of the path is null it will be ignored */
		static Result createProgram(
			const char* vertexShaderPath,
			const char* geometryShaderPath,
			const char* fragmentShaderPath,
			graphics::Context& context,
			ProgramRef& program
		);
	};

}

#endif		// SHADER_LOADER_H
