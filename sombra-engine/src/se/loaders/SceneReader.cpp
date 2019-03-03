#include "se/loaders/SceneReader.h"
#include "GLTFReader.h"

namespace se::loaders {

	SceneReader::SceneReaderUPtr SceneReader::createSceneReader(SceneFileType fileType)
	{
		SceneReaderUPtr ret = nullptr;

		switch (fileType) {
			case SceneFileType::GLTF:
				ret = std::make_unique<GLTFReader>();
				break;
		}

		return ret;
	}

}
