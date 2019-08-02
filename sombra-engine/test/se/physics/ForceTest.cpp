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

	se::physics::RigidBody rb(2.0f, 0.0f, glm::mat3(0.8f), 0.0f, 0.5f);
	rb.position			= glm::vec3(0.0f, 1.0f, 0.0f);
	rb.linearVelocity	= glm::vec3(1.0f, 0.0f, 0.0f);
	rb.angularVelocity	= glm::vec3(0.0f, glm::pi<float>(), 0.0f);
	rb.forceSum			= glm::vec3(0.1f, 0.0f, 0.0f);
	rb.torqueSum		= glm::vec3(0.0f, 0.1f, 0.0f);

	Gravity gravityForce({ 0.0f, -9.8f, 0.0f });
	ForceManager forceManager;
	forceManager.addRBForce(&rb, &gravityForce);
	forceManager.applyForces();

	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(rb.position[i], expectedPosition[i], kTolerance);
		EXPECT_NEAR(rb.linearVelocity[i], expectedLinearVelocity[i], kTolerance);
		EXPECT_NEAR(rb.linearAcceleration[i], expectedLinearAcceleration[i], kTolerance);
		EXPECT_NEAR(rb.forceSum[i], expectedForceSum[i], kTolerance);
		EXPECT_NEAR(rb.angularVelocity[i], expectedAngularVelocity[i], kTolerance);
		EXPECT_NEAR(rb.angularAcceleration[i], expectedAngularAcceleration[i], kTolerance);
		EXPECT_NEAR(rb.torqueSum[i], expectedTorqueSum[i], kTolerance);
	}
	for (int i = 0; i < 4; ++i) {
		EXPECT_NEAR(rb.orientation[i], expectedOrientation[i], kTolerance);
	}
}
