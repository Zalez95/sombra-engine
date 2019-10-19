#ifndef SCENE_READER_H
#define SCENE_READER_H

#include <vector>
#include <memory>
#include "Result.h"
#include "../animation/AnimationNode.h"
#include "../graphics/3D/Camera.h"
#include "../graphics/3D/Renderable3D.h"
#include "../app/Skin.h"

namespace se::loaders {

	/**
	 * Class Scene, holds a single Scene Graph
	 */
	struct Scene
	{
		/**
		 * Struct Entity, holds The data of an Entity of a Scene
		 */
		struct Entity
		{
			/** A pointer to the AnimationNode of the Entity in the Scene
			 * graph */
			animation::AnimationNode* animationNode;

			/** If the Entity has a Camera or not */
			bool hasCamera;

			/** The index of the Camera of the Entity in a Cameras vector */
			std::size_t cameraIndex;

			/** If the Entity has a Renderable3Ds or not */
			bool hasRenderable3Ds;

			/** The index to the Renderable3D indices of the Entity */
			std::size_t renderable3DsIndex;

			/** If the Entity has a Skin or not */
			bool hasSkin;

			/** The index of the Skin of the Entity in a Skins vector */
			std::size_t skinIndex;
		};

		/** The name of the Scene */
		std::string name;

		/** The root AnimationNodes of the Scene */
		std::vector<animation::AnimationNode> rootNodes;

		/** The Entities of the Scene */
		std::vector<Entity> entities;
	};


	/**
	 * Struct Scenes, it holds all the loaded data by the SceneReader
	 */
	struct Scenes
	{
		/** The Scenes loaded by a SceneReader */
		std::vector<std::unique_ptr<Scene>> scenes;

		/** The Cameras loaded by a SceneReader */
		std::vector<std::unique_ptr<graphics::Camera>> cameras;

		/** The indices of an Scene::Entity Renderable3Ds in the renderable3Ds
		 * vector */
		std::vector<std::vector<std::size_t>> renderable3DIndices;

		/** The Renderable3Ds loaded by a SceneReader */
		std::vector<std::unique_ptr<graphics::Renderable3D>> renderable3Ds;

		/** The Skins loaded by a SceneReader */
		std::vector<std::unique_ptr<app::Skin>> skins;
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

		/** Parses the given file and stores the result in the given Scenes
		 * object
		 *
		 * @param	path the path to the file to parse
		 * @param	output the Scenes where the file data will be stored
		 * @return	a Result object with the result of the load operation */
		virtual Result load(const std::string& path, Scenes& output) = 0;
	};

}

#endif		// SCENE_READER_H
