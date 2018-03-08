#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/BoundingBox.h>

#define TOLERANCE 0.000000001f


TEST(BoundingBox, getAABB)
{
	const glm::vec3 lengths(0.5f, 2.0f, 5.5f);
	const fe::collision::BoundingBox bb1(lengths);
	const fe::collision::AABB aabb1 = bb1.getAABB();
	EXPECT_EQ(aabb1.minimum, glm::vec3(-0.25f, -1.0f, -2.75f));
	EXPECT_EQ(aabb1.maximum, glm::vec3(0.25f, 1.0f, 2.75f));
}


TEST(BoundingBox, getAABBTransforms)
{
	const glm::vec3 lengths(0.5f, 2.0f, 5.5f);
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));

	fe::collision::BoundingBox bb1(lengths);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	bb1.setTransforms(t * r);

	const fe::collision::AABB aabb1 = bb1.getAABB();
	glm::vec3 expectedMinimum(3.02625942f, -3.63204646f, -12.16605472f);
	glm::vec3 expectedMaximum(6.97374057f, 1.63204646f, -7.83394575f);
	for (int i = 0; i < 3; ++i) {
		EXPECT_LE(abs(aabb1.minimum[i] - expectedMinimum[i]), TOLERANCE);
		EXPECT_LE(abs(aabb1.maximum[i] - expectedMaximum[i]), TOLERANCE);
	}
}


TEST(BoundingBox, getFurthestPointInDirection)
{
	const glm::vec3 lengths(0.5f, 2.0f, 5.5f);
	glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));

	fe::collision::BoundingBox bb1(lengths);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	bb1.setTransforms(t * r);

	glm::vec3 direction(-0.565685425f, 0.707106781f, 0.424264069f), pointWorld, pointLocal;
	bb1.getFurthestPointInDirection(direction, pointWorld, pointLocal);

	glm::vec3 expectedPWorld(3.38738465f, -2.15441298f, -7.83394575f);
	glm::vec3 expectedPLocal(0.25, 1.0f, 2.75f);
	for (int i = 0; i < 3; ++i) {
		EXPECT_LE(abs(pointWorld[i] - expectedPWorld[i]), TOLERANCE);
		EXPECT_LE(abs(pointLocal[i] - expectedPLocal[i]), TOLERANCE);
	}
}
