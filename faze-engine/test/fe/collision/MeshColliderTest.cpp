#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fe/collision/QuickHull.h>
#include <fe/collision/MeshCollider.h>
#include "TestMeshes.h"

#define TOLERANCE 0.000001f

TEST(MeshCollider, getAABB1)
{
	fe::collision::QuickHull qh(0.0001f);
	fe::collision::HalfEdgeMesh meshData;
	createTestMesh1(meshData);
	qh.calculate(meshData);

	const glm::vec3 expectedMinimum(-0.25f, -1.0f, -2.75f);
	const glm::vec3 expectedMaximum(1.25f, 1.0, 2.75f);

	fe::collision::MeshCollider mc1({ qh.getMesh() });
	fe::collision::AABB aabb1 = mc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], TOLERANCE);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], TOLERANCE);
	}
}


TEST(MeshCollider, getAABBTransforms1)
{
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	fe::collision::QuickHull qh(0.0001f);
	fe::collision::HalfEdgeMesh meshData;
	createTestMesh1(meshData);
	qh.calculate(meshData);

	const glm::vec3 expectedMinimum(3.026389360f, -3.532424926f, -12.166131973f);
	const glm::vec3 expectedMaximum(7.695832729f, 1.698557257f, -7.145406246f);

	fe::collision::MeshCollider mc1({ qh.getMesh() });
	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	mc1.setTransforms(t * r);

	fe::collision::AABB aabb1 = mc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], TOLERANCE);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], TOLERANCE);
	}
}


TEST(MeshCollider, getOverlapingParts1)
{
	fe::collision::HalfEdgeMesh expectedMesh;
	fe::collision::addVertex(expectedMesh, { 1.25f,  1.0f, -2.75f });
	fe::collision::addVertex(expectedMesh, { 1.25f, -1.0f, -2.75f });
	fe::collision::addVertex(expectedMesh, { -0.25f, -1.0f, -2.75f });
	fe::collision::addVertex(expectedMesh, { -0.25f,  1.0f,  0.0f });
	fe::collision::addVertex(expectedMesh, { 1.25f,  1.0f,  2.75f });
	fe::collision::addVertex(expectedMesh, { 1.25f, -1.0f,  2.75f });
	fe::collision::addVertex(expectedMesh, { -0.25f, -1.0f,  0.0f });
	fe::collision::addVertex(expectedMesh, { -0.25f,  1.0f,  2.75f });
	fe::collision::addFace(expectedMesh, { 0, 1, 2 });
	fe::collision::addFace(expectedMesh, { 0, 2, 3 });
	fe::collision::addFace(expectedMesh, { 3, 2, 6, 7 });
	fe::collision::addFace(expectedMesh, { 7, 6, 5 });
	fe::collision::addFace(expectedMesh, { 7, 5, 4 });
	fe::collision::addFace(expectedMesh, { 2, 1, 5, 6 });
	fe::collision::addFace(expectedMesh, { 1, 0, 4, 5 });
	fe::collision::addFace(expectedMesh, { 0, 3, 7, 4 });

	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const fe::collision::AABB aabb1{
		glm::vec3(3.47687816f, -3.09886074f, -10.11952781f),
		glm::vec3(5.47687816f, -1.09886074f, -8.11952781f)
	};

	fe::collision::QuickHull qh(0.0001f);
	fe::collision::HalfEdgeMesh meshData;
	createTestMesh1(meshData);
	qh.calculate(meshData);

	fe::collision::MeshCollider mc1({ qh.getMesh() });
	fe::collision::ConvexPolyhedron expectedRes(expectedMesh);

	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 transforms = t * r;

	mc1.setTransforms(transforms);
	expectedRes.setTransforms(transforms);

	auto result = mc1.getOverlapingParts(aabb1);
	ASSERT_EQ(static_cast<int>(result.size()), 1);

	const std::vector<glm::vec3> testDirections = {
		{  0.526099324f,  0.848074734f, -0.063156284f },
		{  0.847218513f,  0.394129663f, -0.356205850f },
		{  0.549060404f,  0.393087625f, -0.737573623f },
		{ -0.670807957f,  0.684541285f,  0.285341858f },
		{ -0.279787182f, -0.341919273f,  0.897112190f },
		{  0.041331931f, -0.795864343f,  0.604062557f },
		{  0.320478677f, -0.716775774f, -0.619294762f },
		{ -0.672019600f, -0.404401332f,  0.620362162f }
	};
	for (const glm::vec3& dir : testDirections) {
		glm::vec3 pointWorld1, pointWorld2, pointLocal1, pointLocal2;
		result.front()->getFurthestPointInDirection(dir, pointWorld1, pointLocal1);
		expectedRes.getFurthestPointInDirection(dir, pointWorld2, pointLocal2);
		for (int i = 0; i < 3; ++i) {
			EXPECT_NEAR(pointWorld1[i], pointWorld2[i], TOLERANCE);
			EXPECT_NEAR(pointLocal1[i], pointLocal2[i], TOLERANCE);
		}
	}
}
