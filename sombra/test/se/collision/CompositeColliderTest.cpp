#include <gtest/gtest.h>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <se/collision/CompositeCollider.h>
#include <se/collision/BoundingSphere.h>
#include <se/collision/ConvexPolyhedron.h>
#include "TestMeshes.h"

using namespace se::collision;
static constexpr float kTolerance = 0.000001f;

TEST(CompositeCollider, getAABB1)
{
	const glm::vec3 expectedMinimum(-0.25f, -1.0f, -2.75f);
	const glm::vec3 expectedMaximum(1.25f, 1.0, 2.75f);
	const HalfEdgeMesh meshData = createTestMesh2().first;

	std::vector<std::unique_ptr<Collider>> colliders;
	colliders.emplace_back( std::make_unique<ConvexPolyhedron>(meshData) );
	CompositeCollider cc1(colliders);

	AABB aabb1 = cc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(CompositeCollider, getAABBTransforms1)
{
	const glm::vec3 expectedMinimum(3.026389360f, -3.532424926f, -12.166131973f);
	const glm::vec3 expectedMaximum(7.695832729f, 1.698557257f, -7.145406246f);
	const HalfEdgeMesh meshData = createTestMesh2().first;

	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));

	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 transforms = t * r;

	std::vector<std::unique_ptr<Collider>> colliders;
	colliders.emplace_back( std::make_unique<ConvexPolyhedron>(meshData) );
	CompositeCollider cc1(colliders);
	cc1.setTransforms(transforms);

	AABB aabb1 = cc1.getAABB();
	for (int i = 0; i < 3; ++i) {
		EXPECT_NEAR(aabb1.minimum[i], expectedMinimum[i], kTolerance);
		EXPECT_NEAR(aabb1.maximum[i], expectedMaximum[i], kTolerance);
	}
}


TEST(CompositeCollider, updated)
{
	const float radius = 2.0f;
	const HalfEdgeMesh meshData = createTestMesh2().first;

	auto bs1 = std::make_unique<BoundingSphere>(radius);
	BoundingSphere* bs1Ptr = bs1.get();

	std::vector<std::unique_ptr<Collider>> colliders;
	colliders.emplace_back( std::move(bs1) );
	colliders.emplace_back( std::make_unique<ConvexPolyhedron>(meshData) );
	CompositeCollider cc1(colliders);

	EXPECT_TRUE(cc1.updated());
	cc1.resetUpdatedState();
	EXPECT_FALSE(cc1.updated());
	cc1.setTransforms(glm::mat4(1.0f));
	EXPECT_TRUE(cc1.updated());
	cc1.resetUpdatedState();
	EXPECT_FALSE(cc1.updated());
	bs1Ptr->setTransforms(glm::mat4(1.0f));
	EXPECT_TRUE(cc1.updated());
	cc1.resetUpdatedState();
	EXPECT_FALSE(cc1.updated());
}


TEST(CompositeCollider, getOverlapingPartsQH1)
{
	const glm::vec3 translation(5.0f, -1.0f, -10.0f);
	const glm::quat rotation = glm::angleAxis(glm::pi<float>()/3, glm::vec3(2/3.0f, -2/3.0f, 1/3.0f));
	const AABB aabb1{
		glm::vec3(3.47687816f, -3.09886074f, -10.11952781f),
		glm::vec3(5.47687816f, -1.09886074f, -8.11952781f)
	};
	const HalfEdgeMesh meshData = createTestMesh2().first;

	glm::mat4 r = glm::mat4_cast(rotation);
	glm::mat4 t = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 transforms = t * r;

	std::vector<std::unique_ptr<Collider>> colliders;
	colliders.emplace_back( std::make_unique<ConvexPolyhedron>(meshData) );
	CompositeCollider cc1(colliders);
	cc1.setTransforms(transforms);

	auto expectedRes = std::make_unique<ConvexPolyhedron>(meshData);
	expectedRes->setTransforms(transforms);

	cc1.processOverlapingParts(aabb1, [&](const ConvexCollider& part) {
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
			part.getFurthestPointInDirection(dir, pointWorld1, pointLocal1);
			expectedRes->getFurthestPointInDirection(dir, pointWorld2, pointLocal2);
			for (int i = 0; i < 3; ++i) {
				EXPECT_NEAR(pointWorld1[i], pointWorld2[i], kTolerance);
				EXPECT_NEAR(pointLocal1[i], pointLocal2[i], kTolerance);
			}
		}
	});
}
