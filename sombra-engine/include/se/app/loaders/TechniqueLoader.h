#ifndef TECHNIQUE_LOADER_H
#define TECHNIQUE_LOADER_H

#include <memory>
#include "../../graphics/Pass.h"
#include "../../graphics/Technique.h"
#include "../../graphics/core/Program.h"
#include "../graphics/Image.h"
#include "../graphics/Material.h"

namespace se::app {

	class TechniqueLoader
	{
	private:	// Nested types
		using ProgramUPtr = std::unique_ptr<graphics::Program>;
		using ProgramSPtr = std::shared_ptr<graphics::Program>;
		using PassSPtr = std::shared_ptr<graphics::Pass>;

	public:		// Functions
		static void addMaterialBindables(
			PassSPtr pass, const Material& material,
			const ProgramSPtr program
		);

		static void addSplatmapMaterialBindables(
			PassSPtr pass, const SplatmapMaterial& material,
			const ProgramSPtr program
		);

		static void addHeightMapBindables(
			PassSPtr pass,
			const Image<unsigned char>& heightMap, float size, float maxHeight,
			const ProgramSPtr program
		);

		static ProgramUPtr createProgram(
			const char* vertexShaderPath,
			const char* geometryShaderPath,
			const char* fragmentShaderPath
		);
	};

}

#endif		// TECHNIQUE_LOADER_H
