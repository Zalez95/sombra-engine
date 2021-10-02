#include <gtest/gtest.h>
#include <se/physics/RigidBodyWorld.h>
#include <se/physics/constraints/DistanceConstraint.h>
#include <se/physics/constraints/FrictionConstraint.h>

using namespace se::physics;
static constexpr float kTolerance = 0.000001f;

TEST(Constraint, distanceConstraint)
{
	const glm::vec3 expectedPosition1(0.089215789f, 1.022568421f, 0.0f);
	const glm::vec3 expectedLinearVelocity1(0.892157894f, 0.225684210f, 0.0f);
	const glm::quat expectedOrientation1(0.987408638f, 0.0f, 0.155718922f, 0.027855318f);
	const glm::vec3 expectedAngularVelocity1(0.0f, 3.154092653f, 0.564210526f);
	const glm::vec3 expectedPosition2(0.905142105f, 1.988715789f, 0.0f);
	const glm::vec3 expectedLinearVelocity2(-0.948578947f, -0.112842105f, 0.0f);
	const glm::quat expectedOrientation2(0.999998450f, 0.0f, 0.000156249f, 0.001763155f);
	const glm::vec3 expectedAngularVelocity2(0.0f, 0.003125000f, 0.035263157f);

	RigidBodyProperties properties1(2.0f, glm::mat3(0.8f));
	properties1.sleepMotion = 0.5f;

	RigidBodyState state1;
	state1.position			= glm::vec3(0.0f, 1.0f, 0.0f);
	state1.linearVelocity	= glm::vec3(1.0f, 0.0f, 0.0f);
	state1.angularVelocity	= glm::vec3(0.0f, glm::pi<float>(), 0.0f);
	state1.forceSum			= glm::vec3(0.1f, 0.0f, 0.0f);
	state1.torqueSum			= glm::vec3(0.0f, 0.1f, 0.0f);

	RigidBodyProperties properties2(4.0f, glm::mat3(6.4f));
	properties1.sleepMotion = 0.5f;

	RigidBodyState state2;
	state2.position			= glm::vec3(1.0f, 2.0f, 0.0f);
	state2.linearVelocity	= glm::vec3(-1.0f, 0.0f, 0.0f);
	state2.forceSum			= glm::vec3(-0.2f, 0.0f, 0.0f);
	state2.torqueSum			= glm::vec3(0.0f, 0.2f, 0.0f);

	RigidBody rb1(properties1, state1), rb2(properties2, state2);

	DistanceConstraint distanceConstraint({ &rb1, &rb2 });
	distanceConstraint.setAnchorPoints({ glm::vec3(0.5f, 1.0f, 0.0f ), glm::vec3(-1.0f, 1.0f, 0.0f) });

	RigidBodyWorld rigidBodyWorld;
	rigidBodyWorld.getConstraintManager().addConstraint(&distanceConstraint);
	rigidBodyWorld.getConstraintManager().update(0.1f);

	for (std::size_t i = 0; i < 3; ++i) {
		EXPECT_NEAR(rb1.getState().position[i], expectedPosition1[i], kTolerance);
		EXPECT_NEAR(rb1.getState().linearVelocity[i], expectedLinearVelocity1[i], kTolerance);
		EXPECT_NEAR(rb1.getState().angularVelocity[i], expectedAngularVelocity1[i], kTolerance);
		EXPECT_NEAR(rb2.getState().position[i], expectedPosition2[i], kTolerance);
		EXPECT_NEAR(rb2.getState().linearVelocity[i], expectedLinearVelocity2[i], kTolerance);
		EXPECT_NEAR(rb2.getState().angularVelocity[i], expectedAngularVelocity2[i], kTolerance);
	}
	for (std::size_t i = 0; i < 4; ++i) {
		EXPECT_NEAR(rb1.getState().orientation[i], expectedOrientation1[i], kTolerance);
		EXPECT_NEAR(rb2.getState().orientation[i], expectedOrientation2[i], kTolerance);
	}
}


TEST(Constraint, frictionConstraint)
{
	const glm::vec3 expectedPosition1(-0.020233308f, 1.030766725f, 0.010116654f);
	const glm::vec3 expectedLinearVelocity1(-0.202333083f, 0.307666916f, 0.101166541f);
	const glm::quat expectedOrientation1(0.988739013f, 0.013003549f, 0.1485589295f, 0.012503413f);
	const glm::vec3 expectedAngularVelocity1(0.263032998f, 3.005017821f, 0.252916354f);
	const glm::vec3 expectedPosition2(0.910116672f, 1.985616683f, -0.505058348f);
	const glm::vec3 expectedLinearVelocity2(-0.898833458f, -0.143833458f, -0.050583270f);
	const glm::quat expectedOrientation2(0.999871969f, 0.008692886f, -0.0118539361f, -0.006322099f);
	const glm::vec3 expectedAngularVelocity2(0.173879992f, -0.237109074f, -0.126458177f);

	RigidBodyProperties properties1(2.0f, glm::mat3(0.8f));
	properties1.frictionCoefficient = 0.5f;
	properties1.sleepMotion = 0.5f;

	RigidBodyState state1;
	state1.position			= glm::vec3(0.0f, 1.0f, 0.0f);
	state1.linearVelocity	= glm::vec3(0.0f, 1.0f, 0.0f);
	state1.angularVelocity	= glm::vec3(0.0f, glm::pi<float>(), 0.0f);
	state1.forceSum			= glm::vec3(0.0f, -9.8f, 0.0f);

	RigidBodyProperties properties2(4.0f, glm::mat3(6.4f));
	properties1.frictionCoefficient = 0.72f;
	properties1.sleepMotion = 0.5f;

	RigidBodyState state2;
	state2.position			= glm::vec3(1.0f, 2.0f, -0.5f);
	state2.linearVelocity	= glm::vec3(-1.0f, 0.0f, 0.0f);
	state2.forceSum			= glm::vec3(0.0f, -9.8f, 0.0f);

	float mu1 = properties1.frictionCoefficient, mu2 = properties2.frictionCoefficient, mu = std::sqrt(mu1 * mu1 + mu2 * mu2);
	float averageMass = 2.0f / (properties1.invertedMass + properties2.invertedMass);

	RigidBody rb1(properties1, state1), rb2(properties2, state2);

	FrictionConstraint frictionConstraint({ &rb1, &rb2 }, 9.8f, mu);
	frictionConstraint.calculateConstraintBounds(averageMass);
	frictionConstraint.setConstraintVectors({ glm::vec3(0.5f, 1.0f, 0.02f ), glm::vec3(-1.0f, 1.0f, -3.25f) });
	frictionConstraint.setTangent({ 0.666666686f, 0.666666686f, -0.333333343f });

	RigidBodyWorld rigidBodyWorld;
	rigidBodyWorld.getConstraintManager().addConstraint(&frictionConstraint);
	rigidBodyWorld.getConstraintManager().update(0.1f);

	for (std::size_t i = 0; i < 3; ++i) {
		EXPECT_NEAR(rb1.getState().position[i], expectedPosition1[i], kTolerance);
		EXPECT_NEAR(rb1.getState().linearVelocity[i], expectedLinearVelocity1[i], kTolerance);
		EXPECT_NEAR(rb1.getState().angularVelocity[i], expectedAngularVelocity1[i], kTolerance);
		EXPECT_NEAR(rb2.getState().position[i], expectedPosition2[i], kTolerance);
		EXPECT_NEAR(rb2.getState().linearVelocity[i], expectedLinearVelocity2[i], kTolerance);
		EXPECT_NEAR(rb2.getState().angularVelocity[i], expectedAngularVelocity2[i], kTolerance);
	}
	for (std::size_t i = 0; i < 4; ++i) {
		EXPECT_NEAR(rb1.getState().orientation[i], expectedOrientation1[i], kTolerance);
		EXPECT_NEAR(rb2.getState().orientation[i], expectedOrientation2[i], kTolerance);
	}
}
