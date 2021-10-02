#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/physics/collision/TriangleCollider.h>

using namespace se::physics;
static constexpr float kTolerance = 0.000001f;

TEST(TriangleCollider, getAABB)
{
	const TriangleCollider tc1({
		glm::vec3(-1.493818879f, 0.787958443f,-0.043464872f),
		glm::vec3(-1.212138772f, 0.538054049f, 0.669097125f),
		glm::vec3(-1.017795681f,-0.331133425f, 1.551305651f)
	});
	const glm::vec3 expectedMinimum(-1.493818879f, -0.331133425f, -0.043464872f);
	const glm::vec3 expectedMaximum(-1.017795681f, 0.787958443f, 1.551305651f);

	AABB aabb1 = tc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(TriangleCollider, getAABBTransforms)
{
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const glm::vec3 expectedMinimum(3.537716865f, -2.374801635f, -10.685206413f);
	const glm::vec3 expectedMaximum(3.710821151f, -0.500263631f, -9.993264198f);

	TriangleCollider tc1({
		glm::vec3(-1.493818879f, 0.787958443f,-0.043464872f),
		glm::vec3(-1.212138772f, 0.538054049f, 0.669097125f),
		glm::vec3(-1.017795681f,-0.331133425f, 1.551305651f)
	});
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	tc1.setTransforms(t * r);

	AABB aabb1 = tc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(TriangleCollider, updated)
{
	TriangleCollider tc1({
		glm::vec3(-1.493818879f, 0.787958443f,-0.043464872f),
		glm::vec3(-1.212138772f, 0.538054049f, 0.669097125f),
		glm::vec3(-1.017795681f,-0.331133425f, 1.551305651f)
	});
	EXPECT_TRUE(tc1.updated());
	tc1.resetUpdatedState();
	EXPECT_FALSE(tc1.updated());
	tc1.setTransforms(glm::mat4(1.0f));
	EXPECT_TRUE(tc1.updated());
	tc1.resetUpdatedState();
	EXPECT_FALSE(tc1.updated());
}


TEST(TriangleCollider, getFurthestPointInDirection)
{
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const glm::vec3 direction(-0.565685425f, 0.707106781f, 0.424264069f);
	const glm::vec3 expectedPWorld(3.538829803f, -0.500263631f, -10.685206413f);
	const glm::vec3 expectedPLocal(-1.493818879f, 0.787958443f, -0.043464872f);

	TriangleCollider tc1({
		glm::vec3(-1.493818879f, 0.787958443f,-0.043464872f),
		glm::vec3(-1.212138772f, 0.538054049f, 0.669097125f),
		glm::vec3(-1.017795681f,-0.331133425f, 1.551305651f)
	});
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	tc1.setTransforms(t * r);

	glm::vec3 pointWorld, pointLocal;
	tc1.getFurthestPointInDirection(direction, pointWorld, pointLocal);

	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(pointWorld[i], expectedPWorld[i], kTolerance);
		EXPECT_NEAR(pointLocal[i], expectedPLocal[i], kTolerance);
	}
}
