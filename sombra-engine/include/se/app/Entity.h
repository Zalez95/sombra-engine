#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <bitset>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace se::app {

	/** The indices of the managers in the Entity's updated bitset */
	enum class EntityUpdate : int
	{
		Input,		//< The Entity has changed due to the InputManager
		Physics,	//< The Entity has changed due to the PhysicsManager
		Count		//< The number of indices
	};


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

		/** The Entity scale in world space */
		glm::vec3 scale;

		/** If the Entity has been updated by any manager */
		std::bitset< static_cast<int>(EntityUpdate::Count) > updated;

		/** Creates a new Entity
		 *
		 * @param	name the name of the Entity */
		Entity(const std::string& name) :
			name(name),
			position(0.0f), velocity(0.0f),
			orientation(1.0f, glm::vec3(0.0f)), scale(1.0f) {};
	};

}

#endif		// ENTITY_H
