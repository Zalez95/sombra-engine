#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/collision/ConvexPolyhedron.h>
#include "TestMeshes.h"

using namespace se::collision;
static constexpr float kTolerance = 0.000001f;

TEST(ConvexPolyhedron, getAABB)
{
	const HalfEdgeMesh meshData = createTestPolyhedron2();
	const glm::vec3 expectedMinimum(0.0f, 0.0f, 0.0f);
	const glm::vec3 expectedMaximum(1.0, 1.0, 1.6f);

	ConvexPolyhedron cp1(meshData);
	AABB aabb1 = cp1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(ConvexPolyhedron, getAABBTransforms)
{
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const HalfEdgeMesh meshData = createTestPolyhedron2();
	const glm::vec3 expectedMinimum(4.35967969f, -1.70720076f, -10.0f);
	const glm::vec3 expectedMaximum(5.72222232f, -0.21132487f, -8.53376102f);

	ConvexPolyhedron cp1(meshData);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	cp1.setTransforms(t * r);

	AABB aabb1 = cp1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(ConvexPolyhedron, updated)
{
	const HalfEdgeMesh meshData = createTestPolyhedron2();
	ConvexPolyhedron cp1(meshData);
	EXPECT_TRUE(cp1.updated());
	cp1.resetUpdatedState();
	EXPECT_FALSE(cp1.updated());
	cp1.setTransforms(glm::mat4(1.0f));
	EXPECT_TRUE(cp1.updated());
	cp1.resetUpdatedState();
	EXPECT_FALSE(cp1.updated());
}


TEST(ConvexPolyhedron, getFurthestPointInDirection)
{
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const glm::vec3 direction(-0.565685425f, 0.707106781f, 0.424264069f);
	const HalfEdgeMesh meshData = createTestPolyhedron2();
	const glm::vec3 expectedPWorld(4.48910236f, -0.27777779f, -9.53376102f);
	const glm::vec3 expectedPLocal(0.0, 1.0f, 0.0f);

	ConvexPolyhedron cp1(meshData);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	cp1.setTransforms(t * r);

	glm::vec3 pointWorld, pointLocal;
	cp1.getFurthestPointInDirection(direction, pointWorld, pointLocal);

	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(pointWorld[i], expectedPWorld[i], kTolerance);
		EXPECT_NEAR(pointLocal[i], expectedPLocal[i], kTolerance);
	}
}
