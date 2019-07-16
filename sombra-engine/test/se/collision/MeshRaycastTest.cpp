#include <gtest/gtest.h>
#include <se/collision/HalfEdgeMeshRaycast.h>
#include "TestMeshes.h"

using namespace se::collision;
static constexpr float kTolerance = 0.000001f;


TEST(MeshRaycast, closestHitFalse)
{
	auto [mesh, normals] = createTestMesh1();
	const glm::vec3 origin(1.238742113f, 3.651285171f, 3.549336910f), direction(-0.617665886f, -0.526446342f, -0.584245741f);

	HalfEdgeMeshRaycast<8> raycast(kTolerance);
	raycast.buildKDTree(&mesh, &normals);
	auto result = raycast.closestHit(origin, direction);

	EXPECT_FALSE(result.intersects);
}


TEST(MeshRaycast, closestHitVertex)
{
	auto [mesh, normals] = createTestMesh1();
	const glm::vec3 origin(-3.767188549f, -3.157658576f, 1.147563099f), direction(0.619586884f, 0.732411921f, 0.282284826f);
	const int expectedFace = 1;
	const glm::vec3 expectedIntersection(-0.25f, 1.0f, 2.75f);
	const float expectedDistance = 5.676666652f;

	HalfEdgeMeshRaycast<8> raycast(kTolerance);
	raycast.buildKDTree(&mesh, &normals);
	auto result = raycast.closestHit(origin, direction);

	EXPECT_TRUE(result.intersects);
	EXPECT_EQ(result.iFace, expectedFace);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(result.intersection[i], expectedIntersection[i], kTolerance);
	}
	EXPECT_NEAR(result.distance, expectedDistance, kTolerance);
}


TEST(MeshRaycast, closestHitEdge)
{
	auto [mesh, normals] = createTestMesh1();
	const glm::vec3 origin(1.445234775f, -4.599827766f, 0.190760493f), direction(-0.412362039f, 0.875649988f, -0.251385301f);
	const int expectedFace = 11;
	const glm::vec3 expectedIntersection(-0.25f, -1.0f, -0.842693328f);
	const float expectedDistance = 4.111034913f;

	HalfEdgeMeshRaycast<8> raycast(kTolerance);
	raycast.buildKDTree(&mesh, &normals);
	auto result = raycast.closestHit(origin, direction);

	EXPECT_TRUE(result.intersects);
	EXPECT_EQ(result.iFace, expectedFace);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(result.intersection[i], expectedIntersection[i], kTolerance);
	}
	EXPECT_NEAR(result.distance, expectedDistance, kTolerance);
}


TEST(MeshRaycast, closestHitFace)
{
	auto [mesh, normals] = createTestMesh1();
	const glm::vec3 origin(1.093121886f, -0.150672316f, 0.190760493f), direction(-0.180637240f, 0.056110482f, 0.981948018f);
	const int expectedFace = 7;
	const glm::vec3 expectedIntersection(0.833188056f, -0.069930315f, 1.603766441f);
	const float expectedDistance = 1.438982527f;

	HalfEdgeMeshRaycast<8> raycast(kTolerance);
	raycast.buildKDTree(&mesh, &normals);
	auto result = raycast.closestHit(origin, direction);

	EXPECT_TRUE(result.intersects);
	EXPECT_EQ(result.iFace, expectedFace);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(result.intersection[i], expectedIntersection[i], kTolerance);
	}
	EXPECT_NEAR(result.distance, expectedDistance, kTolerance);
}


TEST(MeshRaycast, closestHitInside)
{
	auto [mesh, normals] = createTestMesh1();
	const glm::vec3 origin(0.006464734f, -0.150672316f, 0.190760493f), direction(0.146256104f, -0.019927343f, 0.989045977f);
	const int expectedFace = 1;
	const glm::vec3 expectedIntersection(0.289327859f, -0.189212322f, 2.103601455f);
	const float expectedDistance = 1.934026232f;

	HalfEdgeMeshRaycast<8> raycast(kTolerance);
	raycast.buildKDTree(&mesh, &normals);
	auto result = raycast.closestHit(origin, direction);

	EXPECT_TRUE(result.intersects);
	EXPECT_EQ(result.iFace, expectedFace);
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(result.intersection[i], expectedIntersection[i], kTolerance);
	}
	EXPECT_NEAR(result.distance, expectedDistance, kTolerance);
}
