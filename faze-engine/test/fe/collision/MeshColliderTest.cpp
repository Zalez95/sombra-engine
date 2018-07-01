#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/MeshCollider.h>
#include <fe/collision/ConvexPolyhedron.h>
#include "TestMeshes.h"

#define TOLERANCE 0.000001f

TEST(MeshCollider, getAABB)
{
	fe::collision::HalfEdgeMesh meshData;
	createTestMesh1(meshData);

	const glm::vec3 expectedMinimum(-0.25f, -1.0f, -2.75f);
	const glm::vec3 expectedMaximum(1.25f, 1.0, 2.75f);

	fe::collision::MeshCollider mc1(meshData, fe::collision::ConvexStrategy::QuickHull);
	fe::collision::AABB aabb1 = mc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], TOLERANCE);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], TOLERANCE);
	}
}


TEST(MeshCollider, getAABBTransforms)
{
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	fe::collision::HalfEdgeMesh meshData;
	createTestMesh1(meshData);

	const glm::vec3 expectedMinimum(3.026389360f, -3.532424926f, -12.166131973f);
	const glm::vec3 expectedMaximum(7.695832729f, 1.698557257f, -7.145406246f);

	fe::collision::MeshCollider mc1(meshData, fe::collision::ConvexStrategy::QuickHull);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	mc1.setTransforms(t * r);

	fe::collision::AABB aabb1 = mc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], TOLERANCE);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], TOLERANCE);
	}
}


TEST(MeshCollider, getOverlapingParts)
{
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const fe::collision::AABB aabb1{
		glm::vec3(3.47687816f, -3.09886074f, -10.11952781f),
		glm::vec3(5.47687816f, -1.09886074f, -8.11952781f)
	};

	fe::collision::HalfEdgeMesh meshData;
	createTestMesh1(meshData);
	fe::collision::MeshCollider mc1(meshData, fe::collision::ConvexStrategy::QuickHull);
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 transforms = t * r;

	mc1.setTransforms(transforms);

	auto result = mc1.getOverlapingParts(aabb1);

	ASSERT_EQ(static_cast<int>(result.size()), 1);
}
