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
		/** The indices of the Components that can be updated using the
		 * TransformsComponent */
		enum class Update : int
		{
			Camera = 0,
			Mesh,
			Terrain,
			ParticleSystem,
			RigidBody,
			Collider,
			Animation,
			AudioSource,
			Skin,
			Shadow,
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

		/** The Components that has been updated with the TransformsComponent */
		std::bitset< static_cast<int>(Update::Count) > updated;
	};


	/** Calculates the model matrix of the given TransformsComponent as TRS
	 * matrix
	 *
	 * @param	transforms the TransformsComponent to calculate its model matrix
	 * @return	the model matrix */
	glm::mat4 getModelMatrix(const TransformsComponent& transforms);

}

#endif		// TRANSFORMS_COMPONENT_H
