#ifndef RIGID_BODY_COMPONENT_H
#define RIGID_BODY_COMPONENT_H

#include "../physics/RigidBody.h"
#include "Repository.h"

namespace se::app {

	/**
	 * Class RigidBodyComponent, It's a wrapper for the physics::RigidBody
	 * class, used for mantaining the references to the Repository
	 * physics::Forces
	 */
	class RigidBodyComponent
	{
	private:	// Nested types
		using ForceRef = Repository::ResourceRef<physics::Force>;

	private:	// Attributes
		/** The physics RigidBody */
		physics::RigidBody mRigidBody;

		/** The pointers to the Forces added to the RigidBody */
		std::vector<ForceRef> mForces;

	public:		// Functions
		/** Creates a new RigidBodyComponent
		 *
		 * @param	config the configuration data of the RigidBodyComponent
		 * @param	data the initial movement data of the RigidBodyComponent */
		RigidBodyComponent(
			const physics::RigidBodyProperties& properties = {},
			const physics::RigidBodyState& state = {}
		) : mRigidBody(properties, state) {};

		/** @return	the raw RigidBody of the RigidBodyComponent */
		physics::RigidBody& get() { return mRigidBody; };

		/** @return	the raw audio source of the RigidBodyComponent */
		const physics::RigidBody& get() const { return mRigidBody; };

		/** @return	the RigidBodyProperties of the RigidBodyComponent */
		const physics::RigidBodyProperties& getProperties() const
		{ return mRigidBody.getProperties(); };

		/** Sets the RigidBodyProperties of the RigidBodyComponent
		 *
		 * @param	state the new RigidBodyProperties of the RigidBodyComponent
		 * @return	a reference to the current RigidBodyComponent object */
		RigidBodyComponent& setProperties(
			const physics::RigidBodyProperties& properties
		) { mRigidBody.setProperties(properties); return *this; };

		/** @return	the current RigidBodyState of the RigidBodyComponent */
		const physics::RigidBodyState& getState() const
		{ return mRigidBody.getState(); };

		/** Sets the RigidBodyState of the RigidBodyComponent
		 *
		 * @param	state the new RigidBodyState of the RigidBodyComponent
		 * @return	a reference to the current RigidBodyComponent object */
		RigidBodyComponent& setState(const physics::RigidBodyState& state)
		{ mRigidBody.setState(state); return *this; };

		/** @return	a pointer to the current Collider of the RigidBodyComponent,
		 *			nullptr if it doesn't have one */
		physics::Collider* getCollider() const
		{ return mRigidBody.getCollider(); };

		/** Sets the Collider of the RigidBodyComponent
		 *
		 * @param	collider a pointer to the new Collider of the
		 *			RigidBodyComponent
		 * @return	a reference to the current RigidBodyComponent object */
		RigidBodyComponent& setCollider(
			std::unique_ptr<physics::Collider>&& collider
		) { mRigidBody.setCollider(std::move(collider)); return *this; };

		/** @return	the Collider local trasforms of the RigidBodyComponent */
		const glm::mat4& getColliderLocalTransforms() const
		{ return mRigidBody.getColliderLocalTransforms(); };

		/** Sets the local trasforms matrix of the Collider
		 *
		 * @param	localTransforms the new local transforms matrix of
		 *			the Collider
		 * @return	a reference to the current RigidBodyComponent object */
		RigidBodyComponent& setColliderLocalTrasforms(
			const glm::mat4& localTransforms
		) {
			mRigidBody.setColliderLocalTrasforms(localTransforms);
			return *this;
		};

		/** Adds a Force to the RigidBody
		 *
		 * @param	force the new Force of the RigidBodyComponent
		 * @return	a reference to the current RigidBodyComponent object */
		RigidBodyComponent& addForce(const ForceRef& force)
		{
			mForces.push_back(force);
			mRigidBody.addForce(force.get());
			return *this;
		};

		/** Iterates through all the Forces of the RigidBodyComponent calling
		 * the given callback function
		 *
		 * @param	callback the function to call for each Force */
		template <typename F>
		void processForces(F&& callback) const
		{
			for (auto& force : mForces) {
				callback(force);
			}
		}

		/** Removes the given Force from the RigidBody
		 *
		 * @param	force the Force to remove
		 * @return	a reference to the current RigidBodyComponent object */
		RigidBodyComponent& removeForce(const ForceRef& force)
		{
			mRigidBody.removeForce(force.get());
			mForces.erase(
				std::remove(mForces.begin(), mForces.end(), force),
				mForces.end()
			);
			return *this;
		};
	};

}

#endif		// RIGID_BODY_COMPONENT_H
