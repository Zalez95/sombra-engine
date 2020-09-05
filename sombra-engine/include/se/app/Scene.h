#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include "../utils/Repository.h"
#include "../animation/AnimationNode.h"
#include "Entity.h"
#include "Application.h"

namespace se::app {

	/**
	 * Struct Scene, holds all the common data shared by the Entities of an App
	 * and other data needed for simulating the 3D scene
	 */
	struct Scene
	{
		/** The name of the Scene */
		std::string name;

		/** The application that holds the @see entities data */
		Application& application;

		/** The Entities of the Scene, they will get automatically cleared when
		 * the Scene is destroyed */
		std::vector<Entity> entities;

		/** The root AnimationNode node of the Scene hierarchy */
		animation::AnimationNode rootNode;

		/** The repository that holds all the shared data of the Scene */
		utils::Repository repository;

		/** Creates a new Scene */
		Scene(const char* name, Application& application);
		Scene(const Scene& other) = delete;
		Scene(Scene&& other) = default;

		/** Class destructor */
		~Scene();

		/** Assignment operator */
		Scene& operator=(const Scene& other) = delete;
		Scene& operator=(Scene&& other) = default;
	};

}

#endif		// SCENE_H
