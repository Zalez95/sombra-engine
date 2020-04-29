#ifndef SCENE_READER_H
#define SCENE_READER_H

#include <vector>
#include <memory>
#include "Result.h"
#include "../graphics/Skin.h"
#include "../graphics/Camera.h"
#include "../graphics/Lights.h"
#include "../graphics/Material.h"
#include "../../animation/CompositeAnimator.h"
#include "../../animation/AnimationNode.h"
#include "../../graphics/3D/Mesh.h"
#include "../../graphics/GraphicsEngine.h"

namespace se::app {

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

			/** If the Entity has Primitives or not */
			bool hasPrimitives;

			/** The index to the primitives of the Entity */
			std::size_t primitivesIndex;

			/** If the Entity has a Light or not */
			bool hasLight;

			/** The index of the Light of the Entity in a Lights vector */
			std::size_t lightIndex;

			/** If the Entity has a Skin or not */
			bool hasSkin;

			/** The index of the Skin of the Entity in a Skins vector */
			std::size_t skinIndex;
		};

		/** The name of the Scene */
		std::string name;

		/** The root AnimationNode of the Scene */
		std::unique_ptr<animation::AnimationNode> rootNode;

		/** The Entities of the Scene */
		std::vector<Entity> entities;
	};


	/**
	 * Struct Scenes, it holds all the loaded data by the SceneReader
	 */
	struct Scenes
	{
		/** Maps a Mesh with its respective Material */
		using Primitive = std::pair<std::size_t, std::size_t>;

		/** The Scenes loaded by a SceneReader */
		std::vector<std::unique_ptr<Scene>> scenes;

		/** The Cameras loaded by a SceneReader */
		std::vector<std::unique_ptr<Camera>> cameras;

		/** The Meshes loaded by a SceneReader */
		std::vector<std::unique_ptr<graphics::Mesh>> meshes;

		/** The Meshes loaded by a SceneReader */
		std::vector<std::unique_ptr<Material>> materials;

		/** The Primitives of the Entities */
		std::vector<std::vector<Primitive>> primitives;

		/** The Lights loaded by a SceneReader */
		std::vector<std::unique_ptr<ILight>> lights;

		/** The Skins loaded by a SceneReader */
		std::vector<std::unique_ptr<Skin>> skins;

		/** The CompositeAnimator loaded by a SceneReader */
		std::vector<std::unique_ptr<animation::CompositeAnimator>> animators;
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
	protected:	// Nested types
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
