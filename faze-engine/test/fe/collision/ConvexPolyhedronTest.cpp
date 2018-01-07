#include <fe/collision/ConvexPolyhedron.h>
#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define TOLERANCE 0.000000001


TEST(ConvexPolyhedron, getAABB)
{
	const std::vector<glm::vec3> vertices = {
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.5f, 0.5f, 1.6f }
	};

	const fe::collision::ConvexPolyhedron cp1(vertices);
	const fe::collision::AABB aabb1 = cp1.getAABB();
	EXPECT_EQ(aabb1.mMinimum, glm::vec3(0.0f, 0.0f, 0.0f));
	EXPECT_EQ(aabb1.mMaximum, glm::vec3(1.0, 1.0, 1.6f));
}


TEST(ConvexPolyhedron, getAABBTransforms)
{
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const std::vector<glm::vec3> vertices = {
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.5f, 0.5f, 1.6f }
	};

	fe::collision::ConvexPolyhedron cp1(vertices);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(), translation);
	cp1.setTransforms(t * r);

	const fe::collision::AABB aabb1 = cp1.getAABB();
	glm::vec3 expectedMinimum(4.35967969f, -1.70720076f, -10.0f);
	glm::vec3 expectedMaximum(5.72222232f, -0.21132487f, -8.53376102f);
	for (unsigned int i = 0; i < 3; ++i) {
		EXPECT_LE(abs(aabb1.mMinimum[i] - expectedMinimum[i]), TOLERANCE);
		EXPECT_LE(abs(aabb1.mMaximum[i] - expectedMaximum[i]), TOLERANCE);
	}
}


TEST(ConvexPolyhedron, getFurthestPointInDirection)
{
	glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const std::vector<glm::vec3> vertices = {
		{ 0.0f, 0.0f, 0.0f },
		{ 1.0f, 0.0f, 0.0f },
		{ 1.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.5f, 0.5f, 1.6f }
	};

	fe::collision::ConvexPolyhedron cp1(vertices);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(), translation);
	cp1.setTransforms(t * r);

	glm::vec3 direction(-0.565685425f, 0.707106781f, 0.424264069f), pointWorld, pointLocal;
	cp1.getFurthestPointInDirection(direction, pointWorld, pointLocal);

	glm::vec3 expectedPWorld(4.48910236f, -0.27777779f, -9.53376102f);
	glm::vec3 expectedPLocal(0.0, 1.0f, 0.0f);
	for (unsigned int i = 0; i < 3; ++i) {
		EXPECT_LE(abs(pointWorld[i] - expectedPWorld[i]), TOLERANCE);
		EXPECT_LE(abs(pointLocal[i] - expectedPLocal[i]), TOLERANCE);
	}
}
