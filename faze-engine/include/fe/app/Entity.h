#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace fe { namespace app {

	/**
	 * Struct Entity, a Entity is a game object that holds all the
	 * common data shared by the game Managers
	 */
	struct Entity
	{
		/** The name of the Entity */
		std::string name;

		/** The Entity position in world space */
		glm::vec3 position;

		/** The Entity velocity in world space */
		glm::vec3 velocity;

		/** The Entity orientation in world space */
		glm::quat orientation;

		/** Creates a new Entity
		 *
		 * @param	name the name of the Entity */
		Entity(const std::string& name) :
			name(name),
			position(0.0f), velocity(0.0f),
			orientation(1.0f, glm::vec3(0.0f)) {};

		/** Destructor */
		~Entity() {};
	};

}}

#endif		// ENTITY_H
