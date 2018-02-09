#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/BoundingSphere.h>

#define TOLERANCE 0.000000001


TEST(BoundingSphere, getAABB)
{
	const fe::collision::BoundingSphere bs1(2.0f);
	const fe::collision::AABB aabb1 = bs1.getAABB();
	EXPECT_EQ(aabb1.mMinimum, glm::vec3(-2.0f));
	EXPECT_EQ(aabb1.mMaximum, glm::vec3(2.0f));
}


TEST(BoundingSphere, getAABBZeroRadius)
{
	const fe::collision::BoundingSphere bs1(0.0f);
	const fe::collision::AABB aabb1 = bs1.getAABB();
	EXPECT_EQ(aabb1.mMinimum, glm::vec3(0.0f));
	EXPECT_EQ(aabb1.mMaximum, glm::vec3(0.0f));
}


TEST(BoundingSphere, getAABBTransforms)
{
	const float radius = 1.0f;
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));

	fe::collision::BoundingSphere bs1(radius);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	bs1.setTransforms(t * r);

	const fe::collision::AABB aabb1 = bs1.getAABB();
	EXPECT_EQ(aabb1.mMinimum, glm::vec3(4.0f, -2.0f, -11.0f));
	EXPECT_EQ(aabb1.mMaximum, glm::vec3(6.0f, 0.0f, -9.0f));
}


TEST(BoundingSphere, getCenter)
{
	const float radius = 2.0f;
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));

	fe::collision::BoundingSphere bs1(radius);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	bs1.setTransforms(t * r);

	glm::vec3 center = bs1.getCenter();
	EXPECT_EQ(center, translation);
}


TEST(BoundingSphere, getFurthestPointInDirection)
{
	const float radius = 2.0f;
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));

	fe::collision::BoundingSphere bs1(radius);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	bs1.setTransforms(t * r);

	glm::vec3 direction(-0.565685425f, 0.707106781f, 0.424264069f), pointWorld, pointLocal;
	bs1.getFurthestPointInDirection(direction, pointWorld, pointLocal);

	glm::vec3 expectedPWorld(3.86862915f, 0.414213562f, -9.151471862f);
	glm::vec3 expectedPLocal(-0.13894384f, 1.99500793f, 0.02526247f);
	for (size_t i = 0; i < 3; ++i) {
		EXPECT_LE(abs(pointWorld[i] - expectedPWorld[i]), TOLERANCE);
		EXPECT_LE(abs(pointLocal[i] - expectedPLocal[i]), TOLERANCE);
	}
}
