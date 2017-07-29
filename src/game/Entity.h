#ifndef ENTITY_H
#define ENTITY_H

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace game {

	class InputComponent;
	class PhysicsComponent;
	class GraphicsComponent;


	/**
	 * Class GameEntity, a GameEntity is a game object that holds all the
	 * graphics, physics and other information.
	 */
	class Entity
	{
	public:		// Attributes
		/** the name of the Entity */
		std::string mName;

		/** Common data of the Entity */
		glm::vec3 mPosition;
		glm::quat mOrientation;

		std::unique_ptr<InputComponent> mInputComponent;
		std::unique_ptr<PhysicsComponent> mPhysicsComponent;
		std::unique_ptr<GraphicsComponent> mGraphicsComponent;

		//		/** The parent Entity of the current one */
		//		Entity* mParent;
		//
		//		/** The name of the parent Joint in case of the current Entity is a
		//		 * child of an Skeleton */
		//		std::string mParentJointName;
		//
		//		/** The Children Entities of the current one */
		//		std::vector<Entity*> mChildren;

	public:		// Functions
		/** Class constructor, creates a new Entity
		 *
		 * @param	name the name of the Entity */
		Entity(
			const std::string& name,
			std::unique_ptr<InputComponent> inputComponent,
			std::unique_ptr<PhysicsComponent> physicsComponent,
			std::unique_ptr<GraphicsComponent> graphicsComponent
		);

		/** Class destructor */
		~Entity();

		/** @return	the name of the Entity */
		inline std::string getName() const { return mName; };

		void update(float delta);

		/** @return a pointer to the parent Entity of the current one */
//		inline Entity* getParent() const { return mParent; };
//
//		/** @return the name of the parent joint of the current Entity in case
//		 * that the parent of the current Entity is a Skeleton
//		 * @note if the Parent isn't a Skeleton, the default will always be an
//		 * empty String */
//		inline std::string getParentJointName() const
//		{ return mParentJointName; };
//
//		/** @return the children Entities of the current one */
//		inline std::vector<Entity*> getChildren() const { return mChildren; };
//
//		/** Adds the given Entity as a child of the current one
//		 * 
//		 * @param child a pointer to the Entity that we want to add as a
//		 * children of the current one */
//		inline void addChild(Entity* child)
//		{ if (child) { mChildren.push_back(child); } };
//
//		/** Removes the given Entity as a children of the current one
//		 *
//		 * @param child a pointer to the Entity that we want to remove */
//		inline void removeChild(const Entity* child)
//		{
//			for (auto it = mChildren.begin(); it != mChildren.end(); ++it) {
//				if (*it == child) {
//					mChildren.erase(it);
//					break;
//				}
//			}
//		};
	};

}

#endif		// ENTITY_H
