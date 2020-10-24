#ifndef TRANSFORMS_COMPONENT_H
#define TRANSFORMS_COMPONENT_H

#include <bitset>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace se::app {

	/**
	 * Struct TransformsComponent, it's a Component that holds all the
	 * position, orientation and movement of an Entity
	 */
	struct TransformsComponent
	{
		/** The indices of the managers that can update the Entity's state
		 * in the updated bitset */
		enum class Update : int
		{
			Input,		///< The Entity has changed due to the Input
			Physics,	///< The Entity has changed due to the Physics
			Animation,	///< The Entity has changed due to the Animation
			Count		///< The number of indices
		};

		/** The Entity position in world space */
		glm::vec3 position = glm::vec3(0.0f);

		/** The Entity velocity in world space */
		glm::vec3 velocity = glm::vec3(0.0f);

		/** The Entity orientation in world space */
		glm::quat orientation = glm::quat(1.0f, glm::vec3(0.0f));

		/** The Entity scale in world space */
		glm::vec3 scale = glm::vec3(1.0f);

		/** If the Entity has been updated by any manager */
		std::bitset< static_cast<int>(Update::Count) > updated;
	};

}

#endif		// TRANSFORMS_COMPONENT_H
