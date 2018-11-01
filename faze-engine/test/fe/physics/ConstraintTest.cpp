#include <gtest/gtest.h>
#include <fe/physics/RigidBody.h>
#include <fe/physics/ConstraintManager.h>
#include <fe/physics/constraints/DistanceConstraint.h>

#define TOLERANCE 0.000001

TEST(RigidBody, distanceConstraint)
{
	const glm::vec3 expectedPosition1(0.089215789f, 1.022568421f, 0.0f);
	const glm::vec3 expectedLinearVelocity1(0.892157894f, 0.225684210f, 0.0f);
	const glm::quat expectedOrientation1(0.987408638f, 0.0f, 0.155718922f, 0.027855318f);
	const glm::vec3 expectedAngularVelocity1(0.0f, 3.154092653f, 0.564210526f);
	const glm::vec3 expectedPosition2(0.905142105f, 1.988715789f, 0.0f);
	const glm::vec3 expectedLinearVelocity2(-0.948578947f, -0.112842105f, 0.0f);
	const glm::quat expectedOrientation2(0.999998450f, 0.0f, 0.000156249f, 0.001763155f);
	const glm::vec3 expectedAngularVelocity2(0.0f, 0.003125000f, 0.035263157f);

	fe::physics::RigidBody rb1(2.0f, 0.0f, glm::mat3(0.8f), 0.0f);
	rb1.position		= glm::vec3(0.0f, 1.0f, 0.0f);
	rb1.linearVelocity	= glm::vec3(1.0f, 0.0f, 0.0f);
	rb1.angularVelocity	= glm::vec3(0.0f, glm::pi<float>(), 0.0f);
	rb1.forceSum		= glm::vec3(0.1f, 0.0f, 0.0f);
	rb1.torqueSum		= glm::vec3(0.0f, 0.1f, 0.0f);

	fe::physics::RigidBody rb2(4.0f, 0.0f, glm::mat3(6.4f), 0.0f);
	rb2.position		= glm::vec3(1.0f, 2.0f, 0.0f);
	rb2.linearVelocity	= glm::vec3(-1.0f, 0.0f, 0.0f);
	rb2.forceSum		= glm::vec3(-0.2f, 0.0f, 0.0f);
	rb2.torqueSum		= glm::vec3(0.0f, 0.2f, 0.0f);

	fe::physics::DistanceConstraint distanceConstraint({ &rb1, &rb2 });
	distanceConstraint.setAnchorPoints({ glm::vec3(0.5f, 1.0f, 0.0f ), glm::vec3(-1.0f, 1.0f, 0.0f) });

	fe::physics::ConstraintManager constraintManager;
	constraintManager.addConstraint(&distanceConstraint);
	constraintManager.update(0.1f);

	for (std::size_t i = 0; i < 3; ++i) {
		EXPECT_NEAR(rb1.position[i], expectedPosition1[i], TOLERANCE);
		EXPECT_NEAR(rb1.linearVelocity[i], expectedLinearVelocity1[i], TOLERANCE);
		EXPECT_NEAR(rb1.angularVelocity[i], expectedAngularVelocity1[i], TOLERANCE);
		EXPECT_NEAR(rb2.position[i], expectedPosition2[i], TOLERANCE);
		EXPECT_NEAR(rb2.linearVelocity[i], expectedLinearVelocity2[i], TOLERANCE);
		EXPECT_NEAR(rb2.angularVelocity[i], expectedAngularVelocity2[i], TOLERANCE);
	}
	for (std::size_t i = 0; i < 4; ++i) {
		EXPECT_NEAR(rb1.orientation[i], expectedOrientation1[i], TOLERANCE);
		EXPECT_NEAR(rb2.orientation[i], expectedOrientation2[i], TOLERANCE);
	}
}
