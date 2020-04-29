#ifndef TECHNIQUE_LOADER_H
#define TECHNIQUE_LOADER_H

#include <memory>
#include "../../graphics/Technique.h"
#include "../../graphics/core/Program.h"
#include "../graphics/Material.h"

namespace se::app {

	class TechniqueLoader
	{
	private:	// Nested types
		using ProgramUPtr = std::unique_ptr<graphics::Program>;
		using ProgramSPtr = std::shared_ptr<graphics::Program>;
		using StepSPtr = std::shared_ptr<graphics::Step>;

	public:		// Functions
		static void addMaterialBindables(
			StepSPtr step, const Material& material,
			const ProgramSPtr program
		);

		static void addSplatmapMaterialBindables(
			StepSPtr step, const SplatmapMaterial& material,
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
