#ifndef SCENE_READER_H
#define SCENE_READER_H

#include <vector>
#include <memory>
#include "Result.h"
#include "se/animation/CompositeAnimator.h"
#include "se/animation/AnimationNode.h"
#include "se/graphics/3D/Camera.h"
#include "se/graphics/3D/Lights.h"
#include "se/graphics/3D/Renderable3D.h"
#include "se/graphics/GraphicsEngine.h"
#include "../Skin.h"

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

			/** If the Entity has a Renderable3Ds or not */
			bool hasRenderable3Ds;

			/** The index to the Renderable3D indices of the Entity */
			std::size_t renderable3DsIndex;

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
		/** The Scenes loaded by a SceneReader */
		std::vector<std::unique_ptr<Scene>> scenes;

		/** The Cameras loaded by a SceneReader */
		std::vector<std::unique_ptr<graphics::Camera>> cameras;

		/** The indices of an Scene::Entity Renderable3Ds in the renderable3Ds
		 * vector */
		std::vector<std::vector<std::size_t>> renderable3DIndices;

		/** The Renderable3Ds loaded by a SceneReader */
		std::vector<std::unique_ptr<graphics::Renderable3D>> renderable3Ds;

		/** The Lights loaded by a SceneReader */
		std::vector<std::unique_ptr<graphics::ILight>> lights;

		/** The Skins loaded by a SceneReader */
		std::vector<std::unique_ptr<app::Skin>> skins;

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

	protected:	// Attributes
		/** The GraphicsEngine used for storing Textures and Materials */
		graphics::GraphicsEngine& mGraphicsEngine;

	public:		// Functions
		/** Creates a new SceneReader
		 *
		 * @param	graphicsEngine the GraphicsEngine used for storing
		 *			Textures and Materials */
		SceneReader(graphics::GraphicsEngine& graphicsEngine) :
			mGraphicsEngine(graphicsEngine) {};

		/** Class destructor */
		virtual ~SceneReader() = default;

		/** Creates a SceneReader capable of reading the given file format
		 *
		 * @param	fileType the FileType that we want to read
		 * @param	graphicsEngine the GraphicsEngine used for storing
		 *			Textures and Materials
		 * @return	a pointer to the new SceneReader */
		static SceneReaderUPtr createSceneReader(
			SceneFileType fileType,
			graphics::GraphicsEngine& graphicsEngine
		);

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
