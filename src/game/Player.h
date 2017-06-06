#ifndef PLAYER_H
#define PLAYER_H

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Entity.h"

namespace window {
	struct InputData;
}

namespace game {

	/** Class Player */
	class Player : public Entity
	{
	protected:	// Attributes
		static const float RUN_SPEED;
		static const float JUMP_SPEED;
		static const float MOUSE_SPEED;
		float mFirst;
		const glm::vec2 mWindowDimensions;

	public:		// Functions
		/** Class constructor, creates a new Entity
		 * 
		 * @param	name the name of the Entity
		 * @param	parent a pointer to the parent Entity of the current one
		 * @param	parentJointName the name of the parent joint in case of
		 *			the parent Entity is a Skeleton, empty string by default */
		Player(
			const std::string& name,
			physics::PhysicsEntity* physicsEntity,
			graphics::Camera* camera,
			graphics::PointLight* pointLight,
			graphics::Renderable3D* renderable3D,
			const glm::vec2& windowDimensions
		) : Entity(name, physicsEntity, camera, pointLight, renderable3D),
	   		mWindowDimensions(windowDimensions) {};

		void doInput(const window::InputData* inputData, float delta);
	private:
		void doMouseInput(const window::InputData* inputData, float delta);
		void doKeyboardInput(const window::InputData* inputData, float delta);
	};

}

#endif		// PLAYER_H
