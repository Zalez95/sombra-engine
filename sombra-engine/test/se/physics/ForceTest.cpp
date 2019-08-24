#include <gtest/gtest.h>
#include <se/physics/RigidBody.h>
#include <se/physics/forces/Gravity.h>
#include <se/physics/forces/ForceManager.h>

using namespace se::physics;
static constexpr float kTolerance = 0.000001f;

TEST(Force, gravity)
{
	const glm::vec3 expectedPosition(0.0f, 1.0f, 0.0f);
	const glm::vec3 expectedLinearVelocity(1.0f, 0.0f, 0.0f);
	const glm::vec3 expectedLinearAcceleration(0.0f, 0.0f, 0.0f);
	const glm::vec3 expectedForceSum(0.0f, -19.6f, 0.0f);
	const glm::quat expectedOrientation(1.0f, 0.0f, 0.0f, 0.0f);
	const glm::vec3 expectedAngularVelocity(0.0f, glm::pi<float>(), 0.0f);
	const glm::vec3 expectedAngularAcceleration(0.0f, 0.0f, 0.0f);
	const glm::vec3 expectedTorqueSum(0.0f);

	se::physics::RigidBodyConfig config(2.0f, glm::mat3(0.8f), 0.5f);
	se::physics::RigidBodyData data;
	data.position			= glm::vec3(0.0f, 1.0f, 0.0f);
	data.linearVelocity		= glm::vec3(1.0f, 0.0f, 0.0f);
	data.angularVelocity	= glm::vec3(0.0f, glm::pi<float>(), 0.0f);
	data.forceSum			= glm::vec3(0.1f, 0.0f, 0.0f);
	data.torqueSum			= glm::vec3(0.0f, 0.1f, 0.0f);

	se::physics::RigidBody rb(config, data);

	Gravity gravityForce({ 0.0f, -9.8f, 0.0f });
	ForceManager forceManager;
	forceManager.addRBForce(&rb, &gravityForce);
	forceManager.applyForces();

	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(rb.getData().position[i], expectedPosition[i], kTolerance);
		EXPECT_NEAR(rb.getData().linearVelocity[i], expectedLinearVelocity[i], kTolerance);
		EXPECT_NEAR(rb.getData().linearAcceleration[i], expectedLinearAcceleration[i], kTolerance);
		EXPECT_NEAR(rb.getData().forceSum[i], expectedForceSum[i], kTolerance);
		EXPECT_NEAR(rb.getData().angularVelocity[i], expectedAngularVelocity[i], kTolerance);
		EXPECT_NEAR(rb.getData().angularAcceleration[i], expectedAngularAcceleration[i], kTolerance);
		EXPECT_NEAR(rb.getData().torqueSum[i], expectedTorqueSum[i], kTolerance);
	}
	for (int i = 0; i < 4; ++i) {
		EXPECT_NEAR(rb.getData().orientation[i], expectedOrientation[i], kTolerance);
	}
}
