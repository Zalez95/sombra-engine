#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/collision/Capsule.h>

using namespace se::collision;
static constexpr float kTolerance = 0.000001f;

TEST(Capsule, getAABB)
{
	const Capsule c1(2.0f, 2.0f);
	const glm::vec3 expectedMinimum(-2.0f, -3.0f, -2.0f);
	const glm::vec3 expectedMaximum(2.0f, 3.0f, 2.0f);

	AABB aabb1 = c1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(Capsule, getAABBZeroRadius)
{
	const Capsule c1(0.0f, 0.0f);
	const glm::vec3 expectedMinimum(0.0f);
	const glm::vec3 expectedMaximum(0.0f);

	AABB aabb1 = c1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(Capsule, getAABBTransforms)
{
	const float radius = 1.0f, height = 3.0f;
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const glm::vec3 expectedMinimum(3.233654022f, -3.083333253f, -11.699358940f);
	const glm::vec3 expectedMaximum(6.766345977f, 1.083333253f, -8.300641059f);

	Capsule c1(radius, height);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	c1.setTransforms(t * r);

	AABB aabb1 = c1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(Capsule, updated)
{
	const float radius = 2.0f, height = 1.5f;
	Capsule c1(radius, height);
	EXPECT_TRUE(c1.updated());
	c1.resetUpdatedState();
	EXPECT_FALSE(c1.updated());
	c1.setTransforms(glm::mat4(1.0f));
	EXPECT_TRUE(c1.updated());
	c1.resetUpdatedState();
	EXPECT_FALSE(c1.updated());
}


TEST(Capsule, getFurthestPointInDirection)
{
	const float radius = 2.0f, height = 2.5f;
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const glm::vec3 direction(-0.565685425f, 0.707106781f, 0.424264069f);
	const glm::vec3 expectedPWorld(3.230007171f, 1.316991329f, -8.568673133f);
	const glm::vec3 expectedPLocal(-0.138943731f, 3.245008230f, 0.025262594f);

	Capsule c1(radius, height);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	c1.setTransforms(t * r);

	glm::vec3 pointWorld, pointLocal;
	c1.getFurthestPointInDirection(direction, pointWorld, pointLocal);

	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(pointWorld[i], expectedPWorld[i], kTolerance);
		EXPECT_NEAR(pointLocal[i], expectedPLocal[i], kTolerance);
	}
}
