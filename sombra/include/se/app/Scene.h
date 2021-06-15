#ifndef SCENE_H
#define SCENE_H

#include "../animation/AnimationNode.h"
#include "Repository.h"
#include "Entity.h"
#include "Application.h"

namespace se::app {

	/**
	 * Struct Scene, holds all the common data shared by the Entities of an App
	 * and other data needed for simulating the 3D scene
	 */
	struct Scene
	{
		/** The application that holds the @see entities data */
		Application& application;

		/** The name of the Scene */
		std::string name = {};

		/** The path where the Scene is stored */
		std::string path = {};

		/** The paths to the Scenes linked from the current Scene */
		std::vector<std::string> linkedScenePaths;

		/** The Entities of the Scene, they will get automatically cleared when
		 * the Scene is destroyed */
		std::vector<Entity> entities;

		/** The root AnimationNode node of the Scene hierarchy */
		animation::AnimationNode rootNode;

		/** The repository that holds all the shared data of the Scene. The
		 * available types are Skin, LightSource, ParticleEmitter,
		 * RenderableShader, RenderableShaderStp,
		 * animation::SkeletonAnimator, audio::Buffer,
		 * physics::Force, physics::Constraint, graphics::Program,
		 * graphics::Texture and graphics::Mesh */
		Repository repository;

		/** Creates a new Scene */
		Scene(const std::string& name, Application& application);
		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = default;

		/** Class destructor */
		~Scene();

		/** Assignment operator */
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) = default;
	};


	/** Removes the given Entity hierarchy from the given Scene
	 *
	 * @param	scene the Scene that holds the Entity hierarchy
	 * @param	entity the Entity that has the root AnimationNode of the
	 *			hierarchy to remove */
	void removeEntityHierarchy(Scene& scene, Entity entity);

}

#endif		// SCENE_H
