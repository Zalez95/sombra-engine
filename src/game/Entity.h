#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace game {

	/**
	 * Struct Entity, a Entity is a game object that holds all the
	 * common data shared by the game Managers
	 */
	struct Entity
	{
		/** The name of the Entity */
		std::string mName;

		/** The Entity position in world space */
		glm::vec3 mPosition;

		/** The Entity velocity in world space */
		glm::vec3 mVelocity;
		
		/** The Entity orientation in world space */
		glm::quat mOrientation;

		/** Creates a new Entity
		 *
		 * @param	name the name of the Entity */
		Entity(const std::string& name) : mName(name) {};

		/** Destructor */
		~Entity() {};
	};

}

#endif		// ENTITY_H
