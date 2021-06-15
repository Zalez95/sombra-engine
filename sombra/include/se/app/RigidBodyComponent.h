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
			const physics::RigidBodyConfig& config = physics::RigidBodyConfig(),
			const physics::RigidBodyData& data = physics::RigidBodyData()
		) : mRigidBody(config, data) {};

		/** @return	the raw RigidBody of the RigidBodyComponent */
		physics::RigidBody& get() { return mRigidBody; };

		/** @return	the raw audio source of the RigidBodyComponent */
		const physics::RigidBody& get() const { return mRigidBody; };

		/** @return	the RigidBodyConfig of the RigidBodyComponent */
		const physics::RigidBodyConfig& getConfig() const
		{ return mRigidBody.getConfig(); };

		/** @return	the RigidBodyConfig of the RigidBodyComponent */
		physics::RigidBodyConfig& getConfig()
		{ return mRigidBody.getConfig(); };

		/** @return	the current RigidBodyData of the RigidBodyComponent */
		const physics::RigidBodyData& getData() const
		{ return mRigidBody.getData(); };

		/** @return	the current RigidBodyData of the RigidBodyComponent */
		physics::RigidBodyData& getData()
		{ return mRigidBody.getData(); };

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
		void processForces(F callback) const
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
