#include "se/app/loaders/SceneReader.h"
#include "GLTFReader.h"

namespace se::app {

	SceneReader::SceneReaderUPtr SceneReader::createSceneReader(
		SceneFileType fileType, graphics::GraphicsEngine& graphicsEngine
	) {
		SceneReaderUPtr ret = nullptr;

		switch (fileType) {
			case SceneFileType::GLTF:
				ret = std::make_unique<GLTFReader>(graphicsEngine);
				break;
		}

		return ret;
	}

}
