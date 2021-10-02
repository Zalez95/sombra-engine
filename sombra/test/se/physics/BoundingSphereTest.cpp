#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/physics/collision/BoundingSphere.h>

using namespace se::physics;
static constexpr float kTolerance = 0.000001f;

TEST(BoundingSphere, getAABB)
{
	const BoundingSphere bs1(2.0f);
	const glm::vec3 expectedMinimum(-2.0f);
	const glm::vec3 expectedMaximum(2.0f);

	AABB aabb1 = bs1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(BoundingSphere, getAABBZeroRadius)
{
	const BoundingSphere bs1(0.0f);
	const glm::vec3 expectedMinimum(0.0f);
	const glm::vec3 expectedMaximum(0.0f);

	AABB aabb1 = bs1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(BoundingSphere, getAABBTransforms)
{
	const float radius = 1.0f;
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const glm::vec3 expectedMinimum(4.0f, -2.0f, -11.0f);
	const glm::vec3 expectedMaximum(6.0f, 0.0f, -9.0f);

	BoundingSphere bs1(radius);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	bs1.setTransforms(t * r);

	AABB aabb1 = bs1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(BoundingSphere, getCenter)
{
	const float radius = 2.0f;
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));

	BoundingSphere bs1(radius);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	bs1.setTransforms(t * r);

	glm::vec3 center = bs1.getCenter();
	EXPECT_EQ(center, translation);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(center[i], translation[i], kTolerance);
	}
}


TEST(BoundingSphere, updated)
{
	const float radius = 2.0f;
	BoundingSphere bs1(radius);
	EXPECT_TRUE(bs1.updated());
	bs1.resetUpdatedState();
	EXPECT_FALSE(bs1.updated());
	bs1.setTransforms(glm::mat4(1.0f));
	EXPECT_TRUE(bs1.updated());
	bs1.resetUpdatedState();
	EXPECT_FALSE(bs1.updated());
}


TEST(BoundingSphere, getFurthestPointInDirection)
{
	const float radius = 2.0f;
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const glm::vec3 direction(-0.565685425f, 0.707106781f, 0.424264069f);
	const glm::vec3 expectedPWorld(3.86862915f, 0.414213562f, -9.151471862f);
	const glm::vec3 expectedPLocal(-0.13894384f, 1.99500793f, 0.02526247f);

	BoundingSphere bs1(radius);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	bs1.setTransforms(t * r);

	glm::vec3 pointWorld, pointLocal;
	bs1.getFurthestPointInDirection(direction, pointWorld, pointLocal);

	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(pointWorld[i], expectedPWorld[i], kTolerance);
		EXPECT_NEAR(pointLocal[i], expectedPLocal[i], kTolerance);
	}
}
