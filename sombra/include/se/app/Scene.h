#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include "../animation/AnimationNode.h"
#include "Repository.h"
#include "Entity.h"
#include "Application.h"

namespace se::app {

	/** Struct ResourcePath, it holds a path to a Repository element of type
	 * @tparam T */
	template <typename T>
	struct ResourcePath
	{
		/** The path value, if there are multiple paths they will be
		 * separarated by commas */
		std::string path;

		/** Creates a new ResourcePath
		 *
		 * @param	path the path of the ResourcePath */
		ResourcePath(const std::string& path = "") : path(path) {};
	};


	/**
	 * Struct Scene, holds all the common data shared by the Entities of an App
	 * and other data needed for simulating the 3D scene
	 */
	struct Scene
	{
		using Key = std::string;

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
		Repository repository;

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


	/** Removes the given Entity hierarchy from the given Scene
	 *
	 * @param	scene the Scene that holds the Entity hierarchy
	 * @param	entity the Entity that has the root AnimationNode of the
	 *			hierarchy to remove */
	void removeEntityHierarchy(Scene& scene, Entity entity);

}

#endif		// SCENE_H
