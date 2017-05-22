#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include "../physics/PhysicsEntity.h"
#include "../graphics/3D/Camera.h"
#include "../graphics/3D/Lights.h"
#include "../graphics/3D/Renderable3D.h"

namespace game {

	/**
	 * Class GameEntity, a GameEntity is a game object that holds all the
	 * graphics, physics and other information.
	 */
	class Entity
	{
	protected:	// Attributes
		static const glm::vec3 mDefaultForwardVector;
		static const glm::vec3 mDefaultUpVector;

		/** the name of the Entity */
		std::string mName;

		//		/** The parent Entity of the current one */
		//		Entity* mParent;
		//
		//		/** The name of the parent Joint in case of the current Entity is a
		//		 * child of an Skeleton */
		//		std::string mParentJointName;
		//
		//		/** The Children Entities of the current one */
		//		std::vector<Entity*> mChildren;

		physics::PhysicsEntity* mPhysicsEntity;

		struct
		{
			graphics::Camera* mCamera;
			graphics::PointLight* mPointLight;
			graphics::Renderable3D* mRenderable3D;
		} mGraphicsData;

	public:		// Functions
		/** Class constructor, creates a new Entity
		 *
		 * @param	name the name of the Entity
		 * @param	parent a pointer to the parent Entity of the current one
		 * @param	parentJointName the name of the parent joint in case of
		 *			the parent Entity is a Skeleton, empty string by default */
		Entity(
			const std::string& name,
			physics::PhysicsEntity* physicsEntity,
			graphics::Camera* camera,
			graphics::PointLight* pointLight,
			graphics::Renderable3D* renderable3D
		) : mName(name), mPhysicsEntity(physicsEntity)
			//mParent(parent), mParentJointName(parentJointName) {};
		{
			mGraphicsData.mCamera = camera;
			mGraphicsData.mPointLight = pointLight;
			mGraphicsData.mRenderable3D = renderable3D;
		};

		/** Class destructor */
		~Entity() {};

		/** @return	the name of the Entity */
		inline std::string getName() const { return mName; };

		inline graphics::Camera* getCamera() const { return mGraphicsData.mCamera; };
		inline graphics::PointLight* getPointLight() const { return mGraphicsData.mPointLight; };
		inline graphics::Renderable3D* getRenderable3D() const { return mGraphicsData.mRenderable3D; };

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

		void synch();
	};

}

#endif		// ENTITY_H
