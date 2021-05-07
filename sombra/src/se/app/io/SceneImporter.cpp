#include "se/app/io/SceneImporter.h"
#include "GLTFImporter.h"

namespace se::app {

	SceneImporter::SceneImporterUPtr SceneImporter::createSceneImporter(
		FileType fileType, ShaderBuilder& shaderBuilder
	) {
		SceneImporterUPtr ret = nullptr;

		switch (fileType) {
			case FileType::GLTF:
				ret = std::make_unique<GLTFImporter>(shaderBuilder);
				break;
		}

		return ret;
	}


	void SceneImporter::createDefaultShaders(Scene& scene)
	{
		static const Material defaultMaterial = {
			{ glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), {}, 1.0f, 1.0f, {} },
			{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), graphics::AlphaMode::Opaque, 0.5f, false
		};

		auto defaultShader = scene.repository.find<Scene::Key, RenderableShader>("defaultShader");
		if (!defaultShader) {
			mShaderBuilder.createShader("defaultShader", defaultMaterial, false);
		}

		auto defaultShaderSkin = scene.repository.find<Scene::Key, RenderableShader>("defaultShaderSkin");
		if (!defaultShaderSkin) {
			mShaderBuilder.createShader("defaultShaderSkin", defaultMaterial, false);
		}
	}

}
