#include <gtest/gtest.h>
#include <se/physics/RigidBody.h>
#include <se/physics/forces/Gravity.h>
#include <se/physics/forces/PunctualForce.h>
#include <se/physics/forces/DirectionalForce.h>
#include <se/physics/forces/Gravity.h>
#include <se/physics/PhysicsEngine.h>

using namespace se::physics;
static constexpr float kTolerance = 0.000001f;

TEST(Force, gravity)
{
	const glm::vec3 expectedPosition(0.0f, 1.0f, 0.0f);
	const glm::vec3 expectedLinearVelocity(1.0f, 0.0f, 0.0f);
	const glm::vec3 expectedLinearAcceleration(0.0f, -9.8f, 0.0f);
	const glm::vec3 expectedForceSum(0.0f, -19.6f, 0.0f);
	const glm::quat expectedOrientation(1.0f, 0.0f, 0.0f, 0.0f);
	const glm::vec3 expectedAngularVelocity(0.0f, glm::pi<float>(), 0.0f);
	const glm::vec3 expectedAngularAcceleration(0.0f, 0.0f, 0.0f);
	const glm::vec3 expectedTorqueSum(0.0f);

	se::physics::RigidBodyConfig config(2.0f, glm::mat3(0.8f));
	config.sleepMotion = 0.5f;
	se::physics::RigidBodyData data;
	data.position			= glm::vec3(0.0f, 1.0f, 0.0f);
	data.linearVelocity		= glm::vec3(1.0f, 0.0f, 0.0f);
	data.angularVelocity	= glm::vec3(0.0f, glm::pi<float>(), 0.0f);
	data.forceSum			= glm::vec3(0.1f, 0.0f, 0.0f);
	data.torqueSum			= glm::vec3(0.0f, 0.1f, 0.0f);

	se::physics::RigidBody rb(config, data);

	auto gravityForce = std::make_shared<se::physics::Gravity>(-9.8f);
	rb.addForce(gravityForce);

	se::physics::PhysicsEngine physicsEngine(0.1f, glm::vec3(-1000.f), glm::vec3(1000.0f));
	physicsEngine.addRigidBody(&rb);
	physicsEngine.integrate(0.0f);

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


TEST(Force, punctualForce)
{
	const glm::vec3 expectedPosition(0.0f, 1.0f, 0.0f);
	const glm::vec3 expectedLinearVelocity(1.0f, 0.0f, 0.0f);
	const glm::vec3 expectedLinearAcceleration(-2.5f, 0.6275f, 0.0335f);
	const glm::vec3 expectedForceSum(-5.0f, 1.255f, 0.067f);
	const glm::quat expectedOrientation(1.0f, 0.0f, 0.0f, 0.0f);
	const glm::vec3 expectedAngularVelocity(0.0f, glm::pi<float>(), 0.0f);
	const glm::vec3 expectedAngularAcceleration(10.217813491f, 40.583122253f, 2.346875190f);
	const glm::vec3 expectedTorqueSum(8.174250602f, 32.466499328f, 1.877500057f);

	se::physics::RigidBodyConfig config(2.0f, glm::mat3(0.8f));
	config.sleepMotion = 0.5f;
	se::physics::RigidBodyData data;
	data.position			= glm::vec3(0.0f, 1.0f, 0.0f);
	data.linearVelocity		= glm::vec3(1.0f, 0.0f, 0.0f);
	data.angularVelocity	= glm::vec3(0.0f, glm::pi<float>(), 0.0f);
	data.forceSum			= glm::vec3(0.1f, 0.0f, 0.0f);
	data.torqueSum			= glm::vec3(0.0f, 0.1f, 0.0f);

	se::physics::RigidBody rb(config, data);

	auto punctualForce = std::make_shared<se::physics::PunctualForce>(glm::vec3(-5.0f, 1.255f, 0.067f), glm::vec3(0.5f, 1.25f, -6.5f));
	rb.addForce(punctualForce);

	se::physics::PhysicsEngine physicsEngine(0.1f, glm::vec3(-1000.f), glm::vec3(1000.0f));
	physicsEngine.addRigidBody(&rb);
	physicsEngine.integrate(0.0f);

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


TEST(Force, directionalForce)
{
	const glm::vec3 expectedPosition(0.0f, 1.0f, 0.0f);
	const glm::vec3 expectedLinearVelocity(1.0f, 0.0f, 0.0f);
	const glm::vec3 expectedLinearAcceleration(0.4f, 1.6f, -3.25f);
	const glm::vec3 expectedForceSum(0.8f, 3.2f, -6.5f);
	const glm::quat expectedOrientation(1.0f, 0.0f, 0.0f, 0.0f);
	const glm::vec3 expectedAngularVelocity(0.0f, glm::pi<float>(), 0.0f);
	const glm::vec3 expectedAngularAcceleration(0.0f, 0.0f, 0.0f);
	const glm::vec3 expectedTorqueSum(0.0f);

	se::physics::RigidBodyConfig config(2.0f, glm::mat3(0.8f));
	config.sleepMotion = 0.5f;
	se::physics::RigidBodyData data;
	data.position			= glm::vec3(0.0f, 1.0f, 0.0f);
	data.linearVelocity		= glm::vec3(1.0f, 0.0f, 0.0f);
	data.angularVelocity	= glm::vec3(0.0f, glm::pi<float>(), 0.0f);
	data.forceSum			= glm::vec3(0.1f, 0.0f, 0.0f);
	data.torqueSum			= glm::vec3(0.0f, 0.1f, 0.0f);

	se::physics::RigidBody rb(config, data);

	auto directionalForce = std::make_shared<se::physics::DirectionalForce>(glm::vec3(0.8f, 3.2f, -6.5f));
	rb.addForce(directionalForce);

	se::physics::PhysicsEngine physicsEngine(0.1f, glm::vec3(-1000.f), glm::vec3(1000.0f));
	physicsEngine.addRigidBody(&rb);
	physicsEngine.integrate(0.0f);

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
