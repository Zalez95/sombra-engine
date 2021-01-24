#include <gtest/gtest.h>
#include <se/physics/RigidBody.h>
#include <se/physics/PhysicsEngine.h>
#include <se/physics/constraints/DistanceConstraint.h>

using namespace se::physics;
static constexpr float kTolerance = 0.000001f;

TEST(PhysicsEngine, velocityIntegration)
{
	const float expectedMotion = 3.203016869f;
	const glm::vec3 expectedPosition(-2.96f, 1.968f, -5.008f);
	const glm::quat expectedOrientation(0.066598400f, 0.827548027f, 0.362650245f, -0.423336178f);
	const glm::vec3 expectedLinearVelocity(2.5f, -2.0f, -0.5f);
	const glm::vec3 expectedAngularVelocity(-0.13f, -3.6f, 10.125f);

	RigidBodyConfig config(3.5f, glm::mat3(2.0f / 5.0f * 3.5f * static_cast<float>(std::pow(2.0f, 2))));
	config.linearDrag = 1.0f;
	config.angularDrag = 1.0f;
	config.sleepMotion = 0.5f;

	RigidBodyData data;
	data.position = glm::vec3(-3.0f, 2.0f, -5.0f);
	data.linearVelocity = glm::vec3(2.5f, -2.0f, -0.5f);
	data.orientation = glm::quat(0.020926f, 0.841695f, 0.296882f, -0.450525f);
	data.angularVelocity = glm::vec3(-0.13f, -3.6f, 10.125f);

	RigidBody rb(config, data);
	PhysicsEngine ps(0.2f);
	ps.addRigidBody(&rb);

	ps.resetRigidBodiesState();
	ps.integrate(0.016f);
	ps.checkSleepyRigidBodies(0.016f);

	EXPECT_NEAR(rb.getMotion(), expectedMotion, kTolerance);
	for (std::size_t i = 0; i < 3; ++i) {
		EXPECT_NEAR(rb.getData().position[i], expectedPosition[i], kTolerance);
		EXPECT_NEAR(rb.getData().linearVelocity[i], expectedLinearVelocity[i], kTolerance);
		EXPECT_NEAR(rb.getData().angularVelocity[i], expectedAngularVelocity[i], kTolerance);
	}
	for (std::size_t i = 0; i < 4; ++i) {
		EXPECT_NEAR(rb.getData().orientation[i], expectedOrientation[i], kTolerance);
	}
}


TEST(PhysicsEngine, forceIntegration)
{
	float expectedMotion = 2.930218335f;
	const glm::vec3 expectedPosition(-3.147619724f, 2.891815185f, -5.038386344f);
	const glm::vec3 expectedLinearVelocity(-0.101241126f, -3.636560201f, 10.100842475f);
	const glm::vec3 expectedLinearAcceleration(1.797428965f, -2.285017728f, -1.509840369f);

	RigidBodyConfig config(4.512f, glm::mat3(2.0f / 5.0f * 4.512f * static_cast<float>(std::pow(2.0f, 2))));
	config.linearDrag = 1.0f;
	config.angularDrag = 1.0f;
	config.sleepMotion = 0.5f;

	RigidBodyData data;
	data.position = glm::vec3(-3.146f, 2.95f, -5.2f);
	data.linearVelocity = glm::vec3(-0.13f, -3.6f, 10.125f);
	data.forceSum = glm::vec3(8.11f, -10.31f, -6.8124f);

	RigidBody rb(config, data);
	PhysicsEngine ps(0.2f);
	ps.addRigidBody(&rb);

	ps.resetRigidBodiesState();
	ps.integrate(0.016f);
	ps.checkSleepyRigidBodies(0.016f);

	EXPECT_NEAR(rb.getMotion(), expectedMotion, kTolerance);
	for (std::size_t i = 0; i < 3; ++i) {
		EXPECT_NEAR(rb.getData().position[i], expectedPosition[i], kTolerance);
		EXPECT_NEAR(rb.getData().linearVelocity[i], expectedLinearVelocity[i], kTolerance);
		EXPECT_NEAR(rb.getData().linearAcceleration[i], expectedLinearAcceleration[i], kTolerance);
	}
}


TEST(PhysicsEngine, torqueIntegration)
{
	const float expectedMotion = 4.685478435f;
	const glm::vec3 expectedPosition(5.371876716f, -5.707605838f, 3.908789396f);
	const glm::quat expectedOrientation(0.811599493f, 0.423053562f, -0.389114081f, 0.104509316f);
	const glm::vec3 expectedLinearVelocity(-0.103524908f, -3.650396108f, 10.131346702f);
	const glm::vec3 expectedAngularVelocity(1.044589281f, -1.986972808f, -7.955280303f);
	const glm::vec3 expectedLinearAcceleration(1.654692649f, -3.149759769f, 0.396660745f);
	const glm::vec3 expectedAngularAcceleration(4.974330902f, 1.776694655f, -6.642519950f);

	RigidBodyConfig config(3.953f, glm::mat3(2.0f / 5.0f * 3.953f * static_cast<float>(std::pow(2.0f, 2))));
	config.linearDrag = 1.0f;
	config.angularDrag = 1.0f;
	config.sleepMotion = 0.5f;

	RigidBodyData data;
	data.position = glm::vec3(5.373533248f, -5.649199485f, 3.746687889f);
	data.orientation = glm::quat(0.812893509f, 0.441731840f, -0.347656339f, 0.152355521f);
	data.linearVelocity = glm::vec3(-0.13f, -3.6f, 10.125f);
	data.angularVelocity = glm::vec3(0.965f, -2.0154f, -7.849f);
	data.forceSum = glm::vec3(6.541f, -12.451f, 1.568f);
	data.torqueSum = glm::vec3(31.461647033f, 11.237240791f, -42.012607574f);

	RigidBody rb(config, data);
	PhysicsEngine ps(0.2f);
	ps.addRigidBody(&rb);

	ps.resetRigidBodiesState();
	ps.integrate(0.016f);
	ps.checkSleepyRigidBodies(0.016f);

	EXPECT_NEAR(rb.getMotion(), expectedMotion, kTolerance);
	for (std::size_t i = 0; i < 3; ++i) {
		EXPECT_NEAR(rb.getData().position[i], expectedPosition[i], kTolerance);
		EXPECT_NEAR(rb.getData().linearVelocity[i], expectedLinearVelocity[i], kTolerance);
		EXPECT_NEAR(rb.getData().angularVelocity[i], expectedAngularVelocity[i], kTolerance);
		EXPECT_NEAR(rb.getData().linearAcceleration[i], expectedLinearAcceleration[i], kTolerance);
		EXPECT_NEAR(rb.getData().angularAcceleration[i], expectedAngularAcceleration[i], kTolerance);
	}
	for (std::size_t i = 0; i < 4; ++i) {
		EXPECT_NEAR(rb.getData().orientation[i], expectedOrientation[i], kTolerance);
	}
}


TEST(PhysicsEngine, transformsIntegration)
{
	const float expectedMotion = 2.0f;
	const glm::vec3 expectedPosition(-1.462884187f, 11.208745956f, -14.201886177f);
	const glm::quat expectedOrientation(-0.601788520f, 0.309684604f, 0.276499629f, -0.682271182f);
	const glm::vec3 expectedLinearVelocity(1.977337837f, 5.294228553f, -6.772780418f);
	const glm::vec3 expectedAngularVelocity(0.434944272f, -11.159923553f, -4.147571086f);
	const glm::vec3 expectedLinearAcceleration(1.188229441f, 0.751120924f, -0.517487585f);
	const glm::vec3 expectedAngularAcceleration(1.072137355f, -3.708540916f, -2.921071290f);
	RigidBodyConfig config(6.1781f, glm::mat3(6.1781f * static_cast<float>(std::pow(3.21f, 2) / 6.0f)));
	config.linearDrag = 1.0f;
	config.angularDrag = 1.0f;
	config.sleepMotion = 0.2f;

	RigidBodyData data;
	data.position = glm::vec3(-4.547531127f, 2.949749708f, -3.636348962f);
	data.orientation = glm::quat(-0.074506878f, -0.676165580f, -0.448467493f, -0.579763472f);
	data.linearVelocity = glm::vec3(0.1237f, 4.12248f, -5.9655f);
	data.angularVelocity = glm::vec3(-1.23759f, -5.3746f, 0.4093f);
	data.forceSum = glm::vec3(7.341f, 4.6405f, -3.19709f);
	data.torqueSum = glm::vec3(11.375326156f, -39.347465515f, -30.992446899f);

	RigidBody rb(config, data);
	PhysicsEngine ps(0.2f);
	ps.addRigidBody(&rb);

	ps.resetRigidBodiesState();
	ps.integrate(1.56f);
	ps.checkSleepyRigidBodies(1.56f);

	EXPECT_NEAR(rb.getMotion(), expectedMotion, kTolerance);
	for (std::size_t i = 0; i < 3; ++i) {
		EXPECT_NEAR(rb.getData().position[i], expectedPosition[i], kTolerance);
		EXPECT_NEAR(rb.getData().linearVelocity[i], expectedLinearVelocity[i], kTolerance);
		EXPECT_NEAR(rb.getData().angularVelocity[i], expectedAngularVelocity[i], kTolerance);
		EXPECT_NEAR(rb.getData().linearAcceleration[i], expectedLinearAcceleration[i], kTolerance);
		EXPECT_NEAR(rb.getData().angularAcceleration[i], expectedAngularAcceleration[i], kTolerance);
	}
	for (std::size_t i = 0; i < 4; ++i) {
		EXPECT_NEAR(rb.getData().orientation[i], expectedOrientation[i], kTolerance);
	}
}


TEST(PhysicsEngine, rigidBodyState)
{
	RigidBodyConfig config(6.1781f, glm::mat3(6.1781f * static_cast<float>(std::pow(3.21f, 2)) / 6.0f));
	config.linearDrag = 1.0f;
	config.angularDrag = 1.0f;
	config.sleepMotion = 0.2f;

	RigidBodyData data1;
	data1.position = glm::vec3(-4.547531127f, 2.949749708f, -3.636348962f);
	data1.orientation = glm::quat(-0.074506878f, -0.676165580f, -0.448467493f, -0.579763472f);

	RigidBody rb1(config, data1);

	RigidBodyData data2;
	data2.position = glm::vec3(5.373533248f, -5.649199485f, 3.746687889f);
	data2.orientation = glm::quat(0.812893509f, 0.441731840f, -0.347656339f, 0.152355521f);
	data2.linearVelocity = glm::vec3(0.1237f, 4.12248f, -5.9655f);

	RigidBody rb2(config, data2);

	EXPECT_FALSE(rb1.checkState(RigidBodyState::Sleeping));
	EXPECT_FALSE(rb1.checkState(RigidBodyState::Integrated));
	EXPECT_FALSE(rb1.checkState(RigidBodyState::ConstraintsSolved));
	EXPECT_FALSE(rb2.checkState(RigidBodyState::Sleeping));
	EXPECT_FALSE(rb2.checkState(RigidBodyState::Integrated));
	EXPECT_FALSE(rb2.checkState(RigidBodyState::ConstraintsSolved));

	DistanceConstraint distanceConstraint({ &rb1, &rb2 });
	distanceConstraint.setAnchorPoints({ glm::vec3(0.5f, 1.0f, 0.0f ), glm::vec3(-1.0f, 1.0f, 0.0f) });

	PhysicsEngine ps(0.5f);
	ps.addRigidBody(&rb1);
	ps.addRigidBody(&rb2);
	ps.getConstraintManager().addConstraint(&distanceConstraint);

	EXPECT_FALSE(rb1.checkState(RigidBodyState::Sleeping));
	EXPECT_FALSE(rb1.checkState(RigidBodyState::Integrated));
	EXPECT_FALSE(rb1.checkState(RigidBodyState::ConstraintsSolved));
	EXPECT_FALSE(rb2.checkState(RigidBodyState::Sleeping));
	EXPECT_FALSE(rb2.checkState(RigidBodyState::Integrated));
	EXPECT_FALSE(rb2.checkState(RigidBodyState::ConstraintsSolved));

	ps.resetRigidBodiesState();
	ps.integrate(0.016f);
	ps.solveConstraints(0.016f);
	ps.checkSleepyRigidBodies(0.016f);

	EXPECT_TRUE(rb1.checkState(RigidBodyState::Sleeping));
	EXPECT_TRUE(rb1.checkState(RigidBodyState::Integrated));
	EXPECT_TRUE(rb1.checkState(RigidBodyState::ConstraintsSolved));
	EXPECT_FALSE(rb2.checkState(RigidBodyState::Sleeping));
	EXPECT_TRUE(rb2.checkState(RigidBodyState::Integrated));
	EXPECT_TRUE(rb2.checkState(RigidBodyState::ConstraintsSolved));

	rb2.getData().linearVelocity = glm::vec3(0.0f);
	rb2.synchWithData();

	EXPECT_FALSE(rb2.checkState(RigidBodyState::Sleeping));
	EXPECT_TRUE(rb2.checkState(RigidBodyState::Integrated));
	EXPECT_TRUE(rb2.checkState(RigidBodyState::ConstraintsSolved));

	ps.resetRigidBodiesState();
	ps.integrate(0.016f);
	ps.solveConstraints(0.016f);
	ps.checkSleepyRigidBodies(0.016f);

	EXPECT_TRUE(rb1.checkState(RigidBodyState::Sleeping));
	EXPECT_FALSE(rb1.checkState(RigidBodyState::Integrated));
	EXPECT_TRUE(rb1.checkState(RigidBodyState::ConstraintsSolved));
	EXPECT_TRUE(rb2.checkState(RigidBodyState::Sleeping));
	EXPECT_TRUE(rb2.checkState(RigidBodyState::Integrated));
	EXPECT_TRUE(rb2.checkState(RigidBodyState::ConstraintsSolved));
}
