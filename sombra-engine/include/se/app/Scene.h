#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include "../utils/Repository.h"
#include "../animation/AnimationNode.h"
#include "Entity.h"

namespace se::app {

	/**
	 * Struct Scene, holds all the common data shared by the Entities of an App
	 * and other data needed for simulating the 3D scene
	 */
	struct Scene
	{
		/** The name of the Scene */
		std::string name;

		/** The Entities of the Scene */
		std::vector<Entity> entities;

		/** The root AnimationNode node of the Scene hierarchy */
		animation::AnimationNode rootNode;

		/** The repository that holds all the shared data of the Scene */
		utils::Repository repository;

		/** Creates a new Scene */
		Scene();
	};

}

#endif		// SCENE_H
