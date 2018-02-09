#include <gtest/gtest.h>
#include <fe/physics/RigidBody.h>

#define TOLERANCE 0.000000001


TEST(RigidBody, linearVelocityIntegration)
{
	fe::physics::RigidBody rb(3.5f, 1.0f, glm::mat3(2.0f / 5.0f * 3.5f * pow(2.0f, 2)), 1.0f);
	rb.mPosition = glm::vec3(-3.0f, 2.0f, -5.0f);
	rb.mLinearVelocity = glm::vec3(2.5f, -2.0f, -0.5f);

	rb.integrate(0.016f);

	const glm::vec3 expectedPosition(-2.96f, 1.968f, -5.008f);
	for (size_t i = 0; i < 3; ++i) {
		EXPECT_LE(abs(rb.mPosition[i] - expectedPosition[i]), TOLERANCE);
	}
}


TEST(RigidBody, angularVelocityIntegration)
{
	fe::physics::RigidBody rb(3.5f, 1.0f, glm::mat3(2.0f / 5.0f * 3.5f * pow(2.0f, 2)), 1.0f);
	rb.mOrientation = glm::quat(0.020926f, 0.841695f, 0.296882f, -0.450525f);
	rb.mAngularVelocity = glm::vec3(-0.13f, -3.6f, 10.125f);

	rb.integrate(0.016f);

	const glm::quat expectedOrientation(0.02045356904f, 0.8415004242f, 0.2980528874f, -0.4500772163f);
	for (size_t i = 0; i < 4; ++i) {
		EXPECT_LE(abs(rb.mOrientation[i] - expectedOrientation[i]), TOLERANCE);
	}
}


TEST(RigidBody, linearAccelerationIntegration)
{
	fe::physics::RigidBody rb(3.5f, 1.0f, glm::mat3(2.0f / 5.0f * 3.5f * pow(2.0f, 2)), 1.0f);
	rb.mLinearVelocity = glm::vec3(-0.13f, -3.6f, 10.125f);
	rb.mLinearAcceleration = glm::vec3(78.11f, -10.31f, -6.8124f);

	rb.integrate(0.016f);

	const glm::vec3 expectedLinearVelocity(1.11976f, -3.76496f, 10.0160016f);
	for (size_t i = 0; i < 3; ++i) {
		EXPECT_LE(abs(rb.mLinearVelocity[i] - expectedLinearVelocity[i]), TOLERANCE);
	}
}


TEST(RigidBody, angularAccelerationIntegration)
{
	fe::physics::RigidBody rb(3.5f, 1.0f, glm::mat3(2.0f / 5.0f * 3.5f * pow(2.0f, 2)), 1.0f);
	rb.mAngularVelocity = glm::vec3(5.7912f, -12.126f, -0.0215601f);
	rb.mAngularAcceleration = glm::vec3(0.639f, -9.543f, 33.125711f);

	rb.integrate(0.016f);

	const glm::vec3 expectedAngularVelocity(5.7922224f, -12.278688f, 0.508451276f);
	for (size_t i = 0; i < 3; ++i) {
		EXPECT_LE(abs(rb.mAngularVelocity[i] - expectedAngularVelocity[i]), TOLERANCE);
	}
}
