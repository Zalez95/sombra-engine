#ifndef SCENE_READER_H
#define SCENE_READER_H

#include <vector>
#include <memory>
#include "Result.h"
#include "../app/Scene.h"
#include "../app/Entity.h"
#include "../graphics/3D/Camera.h"
#include "../graphics/3D/Renderable3D.h"

namespace se::loaders {

	/**
	 * Struct DataHolder, it holds all the data loaded by the SceneReader
	 */
	struct DataHolder
	{
		using EntityIndex = std::size_t;
		using Renderable3DIndices = std::vector<std::size_t>;

		std::unique_ptr<app::Scene> scene;
		std::vector<std::unique_ptr<app::Entity>> entities;
		std::vector<std::unique_ptr<graphics::Camera>> cameras;
		std::vector<std::unique_ptr<graphics::Renderable3D>> renderable3Ds;
		std::vector<std::pair<EntityIndex, Renderable3DIndices>> entityR3DMap;
	};


	/** The scenes files than are supported by the SceneReader */
	enum class SceneFileType
	{
		GLTF
	};


	/**
	 * Class SceneReader, it's used to load the full scenes stored in the given
	 * files
	 */
	class SceneReader
	{
	private:	// Nested types
		using SceneReaderUPtr = std::unique_ptr<SceneReader>;

	public:		// Functions
		/** Class destructor */
		virtual ~SceneReader() = default;

		/** Creates a SceneReader capable of reading the given file format
		 *
		 * @param	fileType the FileType that we want to read
		 * @return	a pointer to the new SceneReader */
		static SceneReaderUPtr createSceneReader(SceneFileType fileType);

		/** Parses the Scene located at the given file
		 *
		 * @param	path the path to the file to parse
		 * @param	output the DataHolder where the Scene data will be stored
		 * @return	a Result object with the result of the load operation */
		virtual Result load(const std::string& path, DataHolder& output) = 0;
	};

}

#endif		// SCENE_READER_H
