#ifndef PHYSICS_COMPONENT_H
#define PHYSICS_COMPONENT_H

#include <memory>
#include "../physics/PhysicsEngine.h"
#include "../physics/PhysicsEntity.h"

namespace game {

	class Entity;


	/**
	 * Class PhysicsComponent, it's an asbtract Component used for storing
	 * and update the graphics data of an Entity
	 */
	class PhysicsComponent
	{
	private:	// Attributes
		/** The Engine used for updating the data of the PhysicsComponent */
		physics::PhysicsEngine& mPhysicsEngine;

		/** The physics data that the engine will update */
		std::unique_ptr<physics::PhysicsEntity> mPhysicsEntity;

		glm::vec3 mLastPosition;
		glm::quat mLastOrientation;

	public:		// Functions
		/** Creates a new PhysicsComponent
		 * 
		 * @param	physicsEngine the System used for updating the data of
		 *			the PhysicsComponent
		 * @param	physicsEntity a pointer to physics data of the
		 *			PhysicsComponent */
		PhysicsComponent(
			physics::PhysicsEngine& physicsEngine,
			std::unique_ptr<physics::PhysicsEntity> physicsEntity
		);

		/** Class destructor */
		~PhysicsComponent();

		/** Updates the given common data of the given entity
		 * 
		 * @param	entity the Entity to update
		 * @param	delta the elapsed time since the last update */
		void update(Entity& entity, float delta);
	};

}

#endif		// PHYSICS_COMPONENT_H
