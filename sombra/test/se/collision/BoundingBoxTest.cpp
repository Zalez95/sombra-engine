#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/collision/BoundingBox.h>

using namespace se::collision;
static constexpr float kTolerance = 0.000001f;

TEST(BoundingBox, getAABB)
{
	const glm::vec3 lengths(0.5f, 2.0f, 5.5f);
	const glm::vec3 expectedMinimum(-0.25f, -1.0f, -2.75f);
	const glm::vec3 expectedMaximum(0.25f, 1.0f, 2.75f);

	BoundingBox bb1(lengths);
	AABB aabb1 = bb1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(BoundingBox, getAABBTransforms)
{
	const glm::vec3 lengths(0.5f, 2.0f, 5.5f);
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const glm::vec3 expectedMinimum(3.026389360f, -3.632104396f, -12.166131973f);
	const glm::vec3 expectedMaximum(6.973610401f, 1.632104396f, -7.833868026f);

	BoundingBox bb1(lengths);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	bb1.setTransforms(t * r);

	AABB aabb1 = bb1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(BoundingBox, updated)
{
	const glm::vec3 lengths(0.5f, 2.0f, 5.5f);
	BoundingBox bb1(lengths);
	EXPECT_TRUE(bb1.updated());
	bb1.resetUpdatedState();
	EXPECT_FALSE(bb1.updated());
	bb1.setTransforms(glm::mat4(1.0f));
	EXPECT_TRUE(bb1.updated());
	bb1.resetUpdatedState();
	EXPECT_FALSE(bb1.updated());
}


TEST(BoundingBox, getFurthestPointInDirection)
{
	const glm::vec3 lengths(0.5f, 2.0f, 5.5f);
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const glm::vec3 direction(-0.565685425f, 0.707106781f, 0.424264069f);
	const glm::vec3 expectedPLocal(-0.25, 1.0f, 2.75f);
	const glm::vec3 expectedPWorld(3.026389360f, -2.187659978f, -8.178098678f);

	BoundingBox bb1(lengths);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	bb1.setTransforms(t * r);

	glm::vec3 pointWorld, pointLocal;
	bb1.getFurthestPointInDirection(direction, pointWorld, pointLocal);

	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(pointWorld[i], expectedPWorld[i], kTolerance);
		EXPECT_NEAR(pointLocal[i], expectedPLocal[i], kTolerance);
	}
}
