#include "se/app/loaders/SceneReader.h"
#include "GLTFReader.h"

namespace se::app {

	SceneReader::SceneReaderUPtr SceneReader::createSceneReader(
		FileType fileType,
		Application& application, TechniqueBuilder& techniqueBuilder
	) {
		SceneReaderUPtr ret = nullptr;

		switch (fileType) {
			case FileType::GLTF:
				ret = std::make_unique<GLTFReader>(application, techniqueBuilder);
				break;
		}

		return ret;
	}


	void SceneReader::createDefaultTechniques(Scene& scene)
	{
		static const Material defaultMaterial = {
			{ glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), {}, 1.0f, 1.0f, {} },
			{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), graphics::AlphaMode::Opaque, 0.5f, false
		};

		auto defaultTechnique = scene.repository.find<std::string, graphics::Technique>("defaultTechnique");
		if (!defaultTechnique) {
			defaultTechnique = mTechniqueBuilder.createTechnique(defaultMaterial, false);
			scene.repository.add(std::string("defaultTechnique"), defaultTechnique);
		}

		auto defaultTechniqueSkin = scene.repository.find<std::string, graphics::Technique>("defaultTechniqueSkin");
		if (!defaultTechniqueSkin) {
			defaultTechniqueSkin = mTechniqueBuilder.createTechnique(defaultMaterial, false);
			scene.repository.add(std::string("defaultTechniqueSkin"), defaultTechniqueSkin);
		}
	}

}
